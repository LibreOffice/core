/*************************************************************************
 *
 *  $RCSfile: xmxtrct.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:54 $
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

#include "xmxtrct.hxx"

#include <rtl/memory.h>
#include <tools/zcodec.hxx>
#include <unotools/streamhelper.hxx>
#include <so3/svstor.hxx>

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

    virtual ErrCode                 ReadAt( sal_uInt32 nPos, void* pBuffer, sal_uInt32 nCount, sal_uInt32* pRead ) const;
    virtual ErrCode                 WriteAt( sal_uInt32 nPos, const void* pBuffer, sal_uInt32 nCount, sal_uInt32* pWritten );
    virtual ErrCode                 Flush() const;
    virtual ErrCode                 SetSize( sal_uInt32 nSize );
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

ErrCode XMXLockBytes::ReadAt( sal_uInt32 nPos, void* pBuffer, sal_uInt32 nCount, sal_uInt32* pRead ) const
{
    const sal_uInt32    nSeqLen = maSeq.getLength();
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

ErrCode XMXLockBytes::WriteAt( sal_uInt32 nPos, const void* pBuffer, sal_uInt32 nCount, sal_uInt32* pWritten )
{
    return ERRCODE_IO_CANTWRITE;
}

// ------------------------------------------------------------------------

ErrCode XMXLockBytes::Flush() const
{
    return ERRCODE_NONE;
}

// ------------------------------------------------------------------------

ErrCode XMXLockBytes::SetSize( sal_uInt32 nSize )
{
    return ERRCODE_IO_CANTWRITE;
}

// ------------------------------------------------------------------------

ErrCode XMXLockBytes::Stat( SvLockBytesStat* pStat, SvLockBytesStatFlag eFlag ) const
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
        const String    aStmName( String::CreateFromAscii( "XMLFormat" ) );

        if( !aStorage->GetError() && aStorage->IsStream( aStmName ))
        {
            SvStorageStreamRef xStream( aStorage->OpenStream( aStmName ) );

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
