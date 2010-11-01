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

        inline sal_Bool pack( void *pSource , typelib_TypeDescription *pType );

        sal_Bool packRecursive( void *pSource, typelib_TypeDescription *pType );

        void packTid( const ::rtl::ByteSequence &id, sal_Bool bIgnoreCache = sal_False );
        void packOid( const ::rtl::OUString &oid );
        void packType( void *pSource );

        inline void packCompressedSize( sal_Int32 nSize );
        inline void packInt8( void *pSource );
        inline void packInt16( void *pSource );
        inline void packInt32( void *pSource );
        inline void packString( void *pSource );
        inline sal_Bool packAny( void *pSource );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
