/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

                xRet = new ::utl::OInputStreamHelper( new SvLockBytes( pMemStm, sal_True ), 65535 );
            }
        }
    }

    return xRet;
}
