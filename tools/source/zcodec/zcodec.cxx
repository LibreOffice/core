/*************************************************************************
 *
 *  $RCSfile: zcodec.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:10 $
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

#ifndef _STREAM_HXX
#include "stream.hxx"
#endif
#ifndef _ZLIB_H
#include "zlib/zlib.h"
#endif
#ifndef _ZCODEC_HXX
#include "zcodec.hxx"
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif

// -----------
// - Defines -
// -----------

#define PZSTREAM ((z_stream*) mpsC_Stream)

/* gzip flag byte */
#define GZ_ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define GZ_HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define GZ_EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define GZ_ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define GZ_COMMENT      0x10 /* bit 4 set: file comment present */
#define GZ_RESERVED     0xE0 /* bits 5..7: reserved */

static int gz_magic[2] = { 0x1f, 0x8b }; /* gzip magic header */


// ----------
// - ZCodec -
// ----------

ZCodec::ZCodec( ULONG nInBufSize, ULONG nOutBufSize, ULONG nMemUsage )
{
    mnMemUsage = nMemUsage;
    mnInBufSize = nInBufSize;
    mnOutBufSize = nOutBufSize;
    mpsC_Stream = new z_stream;
}

ZCodec::ZCodec( void )
{
    mnMemUsage = MAX_MEM_USAGE;
    mnInBufSize = DEFAULT_IN_BUFSIZE;
    mnOutBufSize = DEFAULT_OUT_BUFSIZE;
    mpsC_Stream = new z_stream;
}

// ------------------------------------------------------------------------

ZCodec::~ZCodec()
{
    delete (z_stream*) mpsC_Stream;
}

// ------------------------------------------------------------------------

void ZCodec::BeginCompression( ULONG nCompressMethod )
{
    mbInit = 0;
    mbStatus = TRUE;
    mbFinish = FALSE;
    mpIStm = mpOStm = NULL;
    mnInToRead = 0xffffffff;
    mpInBuf = mpOutBuf = NULL;
    PZSTREAM->total_out = PZSTREAM->total_in = 0;
    mnCompressMethod = nCompressMethod;
    PZSTREAM->zalloc = ( alloc_func )0;
    PZSTREAM->zfree = ( free_func )0;
    PZSTREAM->opaque = ( voidpf )0;
    PZSTREAM->avail_out = PZSTREAM->avail_in = 0;
}

// ------------------------------------------------------------------------

long ZCodec::EndCompression()
{
    long retvalue;

    if ( mbInit != 0 )
    {
        if ( mbInit & 2 )   // 1->decompress, 3->compress
        {
            do
            {
                ImplWriteBack();
            }
            while ( deflate( PZSTREAM, Z_FINISH ) != Z_STREAM_END );

            ImplWriteBack();

            retvalue = PZSTREAM->total_in;
            deflateEnd( PZSTREAM );
        }
        else
        {
            retvalue = PZSTREAM->total_out;
            inflateEnd( PZSTREAM );
        }
        delete[] mpOutBuf;
        delete[] mpInBuf;
    }
    return ( mbStatus ) ? retvalue : -1;
}


// ------------------------------------------------------------------------

long ZCodec::Compress( SvStream& rIStm, SvStream& rOStm )
{
    char err;
    long nOldTotal_In = PZSTREAM->total_in;

    if ( mbInit == 0 )
    {
        mpIStm = &rIStm;
        mpOStm = &rOStm;
        ImplInitBuf( FALSE );
        mpInBuf = new BYTE[ mnInBufSize ];
    }
    while (( PZSTREAM->avail_in = mpIStm->Read( PZSTREAM->next_in = mpInBuf, mnInBufSize )) != 0 )
    {
        if ( PZSTREAM->avail_out == 0 )
            ImplWriteBack();
        err = deflate( PZSTREAM, Z_NO_FLUSH );
        if ( err < 0 )
        {
            mbStatus = FALSE;
            break;
        }
    };
    return ( mbStatus ) ? PZSTREAM->total_in - nOldTotal_In : -1;
}

// ------------------------------------------------------------------------

long ZCodec::Decompress( SvStream& rIStm, SvStream& rOStm )
{
    char    err;
    ULONG   nInToRead;
    long    nOldTotal_Out = PZSTREAM->total_out;

    if ( mbFinish )
        return PZSTREAM->total_out - nOldTotal_Out;

    if ( mbInit == 0 )
    {
        mpIStm = &rIStm;
        mpOStm = &rOStm;
        ImplInitBuf( TRUE );
        PZSTREAM->next_out = mpOutBuf = new BYTE[ PZSTREAM->avail_out = mnOutBufSize ];
    }
    do
    {
        if ( PZSTREAM->avail_out == 0 ) ImplWriteBack();
        if ( PZSTREAM->avail_in == 0 && mnInToRead )
        {
            nInToRead = ( mnInBufSize > mnInToRead ) ? mnInToRead : mnInBufSize;
            PZSTREAM->avail_in = mpIStm->Read( PZSTREAM->next_in = mpInBuf, nInToRead );
            mnInToRead -= nInToRead;

            if ( mnCompressMethod & ZCODEC_UPDATE_CRC )
                mnCRC = UpdateCRC( mnCRC, mpInBuf, nInToRead );

        }
        err = inflate( PZSTREAM, Z_NO_FLUSH );
        if ( err < 0 )
        {
            mbStatus = FALSE;
            break;
        }

    }
    while ( ( err != Z_STREAM_END)  && ( PZSTREAM->avail_in || mnInToRead ) );
    ImplWriteBack();

    if ( err == Z_STREAM_END )
        mbFinish = TRUE;
    return ( mbStatus ) ? PZSTREAM->total_out - nOldTotal_Out : -1;
}

// ------------------------------------------------------------------------

long ZCodec::Write( SvStream& rOStm, const BYTE* pData, ULONG nSize )
{
    if ( mbInit == 0 )
    {
        mpOStm = &rOStm;
        ImplInitBuf( FALSE );
    }

    PZSTREAM->avail_in = nSize;
    PZSTREAM->next_in = (unsigned char*)pData;

    while ( PZSTREAM->avail_in || ( PZSTREAM->avail_out == 0 ) )
    {
        if ( PZSTREAM->avail_out == 0 )
            ImplWriteBack();

        if ( deflate( PZSTREAM, Z_NO_FLUSH ) < 0 )
        {
            mbStatus = FALSE;
            break;
        }
    }
    return ( mbStatus ) ? nSize : -1;
}

// ------------------------------------------------------------------------

long ZCodec::Read( SvStream& rIStm, BYTE* pData, ULONG nSize )
{
    char    err;
    ULONG   nInToRead;

    if ( mbFinish )
        return 0;           // PZSTREAM->total_out;

    mpIStm = &rIStm;
    if ( mbInit == 0 )
    {
        ImplInitBuf( TRUE );
    }
    PZSTREAM->avail_out = nSize;
    PZSTREAM->next_out = pData;
    do
    {
        if ( PZSTREAM->avail_in == 0 && mnInToRead )
        {
            nInToRead = (mnInBufSize > mnInToRead) ? mnInToRead : mnInBufSize;
            PZSTREAM->avail_in = mpIStm->Read (
                PZSTREAM->next_in = mpInBuf, nInToRead);
            mnInToRead -= nInToRead;

            if ( mnCompressMethod & ZCODEC_UPDATE_CRC )
                mnCRC = UpdateCRC( mnCRC, mpInBuf, nInToRead );

        }
        err = inflate( PZSTREAM, Z_NO_FLUSH );
        if ( err < 0 )
        {
            // Accept Z_BUF_ERROR as EAGAIN or EWOULDBLOCK.
            mbStatus = (err == Z_BUF_ERROR);
            break;
        }
    }
    while ( (err != Z_STREAM_END) &&
            (PZSTREAM->avail_out != 0) &&
            (PZSTREAM->avail_in || mnInToRead) );
    if ( err == Z_STREAM_END )
        mbFinish = TRUE;

    return (mbStatus ? nSize - PZSTREAM->avail_out : -1);
}

// ------------------------------------------------------------------------

#pragma optimize ("",off)

long ZCodec::ReadAsynchron( SvStream& rIStm, BYTE* pData, ULONG nSize )
{
    char    err;
    ULONG   nInToRead;

    if ( mbFinish )
        return 0;           // PZSTREAM->total_out;

    if ( mbInit == 0 )
    {
        mpIStm = &rIStm;
        ImplInitBuf( TRUE );
    }
    PZSTREAM->avail_out = nSize;
    PZSTREAM->next_out = pData;
    do
    {
        if ( PZSTREAM->avail_in == 0 && mnInToRead )
        {
            nInToRead = (mnInBufSize > mnInToRead) ? mnInToRead : mnInBufSize;

            ULONG nStreamPos = rIStm.Tell();
            rIStm.Seek( STREAM_SEEK_TO_END );
            ULONG nMaxPos = rIStm.Tell();
            rIStm.Seek( nStreamPos );
            if ( ( nMaxPos - nStreamPos ) < nInToRead )
            {
                rIStm.SetError( ERRCODE_IO_PENDING );
                break;
            }

            PZSTREAM->avail_in = mpIStm->Read (
                PZSTREAM->next_in = mpInBuf, nInToRead);
            mnInToRead -= nInToRead;

            if ( mnCompressMethod & ZCODEC_UPDATE_CRC )
                mnCRC = UpdateCRC( mnCRC, mpInBuf, nInToRead );

        }
        err = inflate( PZSTREAM, Z_NO_FLUSH );
        if ( err < 0 )
        {
            // Accept Z_BUF_ERROR as EAGAIN or EWOULDBLOCK.
            mbStatus = (err == Z_BUF_ERROR);
            break;
        }
    }
    while ( (err != Z_STREAM_END) &&
            (PZSTREAM->avail_out != 0) &&
            (PZSTREAM->avail_in || mnInToRead) );
    if ( err == Z_STREAM_END )
        mbFinish = TRUE;

    return (mbStatus ? nSize - PZSTREAM->avail_out : -1);
}

#pragma optimize ("",on)

// ------------------------------------------------------------------------

void ZCodec::ImplWriteBack()
{
    ULONG nAvail = mnOutBufSize - PZSTREAM->avail_out;

    if ( nAvail )
    {
        if ( mbInit & 2 && ( mnCompressMethod & ZCODEC_UPDATE_CRC ) )
            mnCRC = UpdateCRC( mnCRC, mpOutBuf, nAvail );
        mpOStm->Write( PZSTREAM->next_out = mpOutBuf, nAvail );
        PZSTREAM->avail_out = mnOutBufSize;
    }
}

// ------------------------------------------------------------------------

void ZCodec::SetBreak( ULONG nInToRead )
{
    mnInToRead = nInToRead;
}

// ------------------------------------------------------------------------

ULONG ZCodec::GetBreak( void )
{
    return ( mnInToRead + PZSTREAM->avail_in );
}

// ------------------------------------------------------------------------

void ZCodec::SetCRC( ULONG nCRC )
{
    mnCRC = nCRC;
}

// ------------------------------------------------------------------------

ULONG ZCodec::GetCRC()
{
    return mnCRC;
}

// ------------------------------------------------------------------------

void ZCodec::ImplInitBuf ( BOOL nIOFlag )
{
    if ( mbInit == 0 )
    {
        if ( nIOFlag )
        {
            mbInit = 1;
            if ( mbStatus && ( mnCompressMethod & ZCODEC_GZ_LIB ) )
            {
                BYTE n1, n2, j, nMethod, nFlags;
                for ( int i = 0; i < 2; i++ )   // gz - magic number
                {
                    *mpIStm >> j;
                    if ( j != gz_magic[ i ] )
                        mbStatus = FALSE;
                }
                *mpIStm >> nMethod;
                *mpIStm >> nFlags;
                if ( nMethod != Z_DEFLATED )
                    mbStatus = FALSE;
                if ( ( nFlags & GZ_RESERVED ) != 0 )
                    mbStatus = FALSE;
                /* Discard time, xflags and OS code: */
                mpIStm->SeekRel( 6 );
                /* skip the extra field */
                if ( nFlags & GZ_EXTRA_FIELD )
                {
                    *mpIStm >> n1 >> n2;
                    mpIStm->SeekRel( n1 + ( n2 << 8 ) );
                }
                /* skip the original file name */
                if ( nFlags & GZ_ORIG_NAME)
                {
                    do
                    {
                        *mpIStm >> j;
                    }
                    while ( j && !mpIStm->IsEof() );
                }
                /* skip the .gz file comment */
                if ( nFlags & GZ_COMMENT )
                {
                    do
                    {
                        *mpIStm >> j;
                    }
                    while ( j && !mpIStm->IsEof() );
                }
                /* skip the header crc */
                if ( nFlags & GZ_HEAD_CRC )
                    mpIStm->SeekRel( 2 );
                if ( mbStatus )
                    mbStatus = ( inflateInit2( PZSTREAM, -MAX_WBITS) != Z_OK ) ? FALSE : TRUE;
            }
            else
            {
                mbStatus = ( inflateInit( PZSTREAM ) >= 0 );
            }
            mpInBuf = new BYTE[ mnInBufSize ];
        }
        else
        {
            mbInit = 3;

            mbStatus = ( deflateInit2_( PZSTREAM, mnCompressMethod & 0xff, Z_DEFLATED,
                MAX_WBITS, mnMemUsage, ( mnCompressMethod >> 8 ) & 0xff,
                    ZLIB_VERSION, sizeof( z_stream ) ) >= 0 );

            PZSTREAM->next_out = mpOutBuf = new BYTE[ PZSTREAM->avail_out = mnOutBufSize ];
        }
    }
}

// ------------------------------------------------------------------------

ULONG ZCodec::UpdateCRC ( ULONG nLatestCRC, ULONG nNumber )
{

#ifdef __LITTLEENDIAN
    nNumber = SWAPLONG( nNumber );
#endif
    return rtl_crc32( nLatestCRC, &nNumber, 4 );
}

// ------------------------------------------------------------------------

ULONG ZCodec::UpdateCRC ( ULONG nLatestCRC, BYTE* pSource, long nDatSize)
{
    return rtl_crc32( nLatestCRC, pSource, nDatSize );
}

// ------------------------------------------------------------------------

void GZCodec::BeginCompression( ULONG nCompressMethod )
{
    ZCodec::BeginCompression( nCompressMethod | ZCODEC_GZ_LIB );
};


