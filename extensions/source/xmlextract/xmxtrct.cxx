/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmxtrct.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:06:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "xmxtrct.hxx"

#include <rtl/memory.h>
#include <tools/zcodec.hxx>
#include <unotools/streamhelper.hxx>
#include <sot/storage.hxx>

// ----------------
// - XMXLockBytes -
// ----------------

class XMXLockBytes : public SvLockBytes
{
    REF( NMSP_IO::XInputStream )    mxIStm;
    SEQ( sal_Int8 )                 maSeq;

                                    XMXLockBytes();

public:

                                    XMXLockBytes( const REF( NMSP_IO::XInputStream )& rxIStm );
    virtual                         ~XMXLockBytes();

    virtual ErrCode                 ReadAt( sal_Size nPos, void* pBuffer, sal_Size nCount, sal_Size* pRead ) const;
    virtual ErrCode                 WriteAt( sal_Size nPos, const void* pBuffer, sal_Size nCount, sal_Size* pWritten );
    virtual ErrCode                 Flush() const;
    virtual ErrCode                 SetSize( sal_Size nSize );
    virtual ErrCode                 Stat( SvLockBytesStat*, SvLockBytesStatFlag ) const;
};

// ------------------------------------------------------------------------

XMXLockBytes::XMXLockBytes( const REF( NMSP_IO::XInputStream )& rxIStm ) :
    mxIStm( rxIStm )
{
    if( mxIStm.is() )
    {
        const sal_uInt32    nBytesToRead = 65535;
        sal_uInt32          nRead;

        do
        {
            SEQ( sal_Int8 ) aReadSeq;

            nRead = mxIStm->readSomeBytes( aReadSeq, nBytesToRead );

            if( nRead )
            {
                const sal_uInt32 nOldLength = maSeq.getLength();
                maSeq.realloc( nOldLength + nRead );
                rtl_copyMemory( maSeq.getArray() + nOldLength, aReadSeq.getConstArray(), aReadSeq.getLength() );
            }
        }
        while( nBytesToRead == nRead );
    }
}

// ------------------------------------------------------------------------

XMXLockBytes::~XMXLockBytes()
{
}

// ------------------------------------------------------------------------

ErrCode XMXLockBytes::ReadAt( sal_Size nPos, void* pBuffer, sal_Size nCount, sal_Size* pRead ) const
{
    const sal_Size      nSeqLen = maSeq.getLength();
    ErrCode             nErr = ERRCODE_NONE;

    if( nPos < nSeqLen )
    {
        if( ( nPos + nCount ) > nSeqLen )
            nCount = nSeqLen - nPos;

        rtl_copyMemory( pBuffer, maSeq.getConstArray() + nPos, nCount );
        *pRead = nCount;
    }
    else
        *pRead = 0UL;

    return nErr;
}

// ------------------------------------------------------------------------

ErrCode XMXLockBytes::WriteAt( sal_Size /*nPos*/, const void* /*pBuffer*/, sal_Size /*nCount*/, sal_Size* /*pWritten*/ )
{
    return ERRCODE_IO_CANTWRITE;
}

// ------------------------------------------------------------------------

ErrCode XMXLockBytes::Flush() const
{
    return ERRCODE_NONE;
}

// ------------------------------------------------------------------------

ErrCode XMXLockBytes::SetSize( sal_Size /*nSize*/ )
{
    return ERRCODE_IO_CANTWRITE;
}

// ------------------------------------------------------------------------

ErrCode XMXLockBytes::Stat( SvLockBytesStat* pStat, SvLockBytesStatFlag /*eFlag*/ ) const
{
    pStat->nSize = maSeq.getLength();
    return ERRCODE_NONE;
}

// ----------------
// - XMLExtractor -
// ----------------

XMLExtractor::XMLExtractor( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr ) :
    mxFact( rxMgr )
{
}

// -----------------------------------------------------------------------------

XMLExtractor::~XMLExtractor()
{
}

// -----------------------------------------------------------------------------

REF( NMSP_IO::XInputStream ) SAL_CALL XMLExtractor::extract( const REF( NMSP_IO::XInputStream )& rxIStm ) throw( NMSP_UNO::RuntimeException )
{
    REF( NMSP_IO::XInputStream ) xRet;

    if( rxIStm.is() )
    {
        SvStream        aIStm( new XMXLockBytes( rxIStm ) );
        SvStorageRef    aStorage( new SvStorage( aIStm ) );
        String          aStmName;
        const String    aFormat1( String::CreateFromAscii( "XMLFormat" ) );
        const String    aFormat2( String::CreateFromAscii( "XMLFormat2" ) );

        if( aStorage->IsContained( aFormat2 ) )
            aStmName = aFormat2;
        else if( aStorage->IsContained( aFormat1 ) )
            aStmName = aFormat1;

        if( !aStorage->GetError() && aStmName.Len() && aStorage->IsStream( aStmName ) )
        {
            SvStorageStreamRef xStream( aStorage->OpenSotStream( aStmName ) );

            if( xStream.Is() )
            {
                SvMemoryStream* pMemStm = new SvMemoryStream( 65535, 65535 );
                ZCodec          aCodec;

                aCodec.BeginCompression( ZCODEC_BEST_COMPRESSION );
                aCodec.Decompress( *xStream, *pMemStm );
                aCodec.EndCompression();

                xRet = new ::utl::OInputStreamHelper( new SvLockBytes( pMemStm, TRUE ), 65535 );
            }
        }
    }

    return xRet;
}
