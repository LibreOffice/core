/*************************************************************************
 *
 *  $RCSfile: urp_marshal_decl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:50 $
 *
 *  Copyright according the GNU Public License.
 *
 ************************************************************************/
#ifndef _URP_MARSHAL_DECL_HXX_
#define _URP_MARSHAL_DECL_HXX_

/**************************************************************************
#*
#*    last change   $Author: hr $ $Date: 2000-09-18 15:28:50 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile: $
#*
#*    Copyright (c) 2000, Sun Microsystems
#*
#************************************************************************/
#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>

#include <com/sun/star/uno/Type.hxx>

namespace bridges_urp
{
    struct urp_BridgeImpl;

      typedef void ( SAL_CALL * urp_extractOidCallback ) (
          remote_Interface *pRemoteI,
          rtl_uString **ppOid );

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

        void packTid( const ::rtl::ByteSequence &id );
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
        inline void finish();

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
        sal_Int32 m_nSizeOffset;
        struct urp_BridgeImpl *m_pBridgeImpl;
        urp_extractOidCallback m_callback;
    };
}
#endif
