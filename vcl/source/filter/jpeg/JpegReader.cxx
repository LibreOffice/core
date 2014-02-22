/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/config.h>

#include "stdio.h"
#include "jpeg.h"
#include <jpeglib.h>
#include <jerror.h>

#include "JpegReader.hxx"
#include <vcl/bmpacc.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/graphicfilter.hxx>

#define JPEG_MIN_READ 512
#define BUFFER_SIZE  4096
namespace {
    
    
    
    
    static const sal_uInt64 MAX_BITMAP_BYTE_SIZE = sal_uInt64(512 * 1024 * 1024);
}

/* Expanded data source object for stdio input */

struct SourceManagerStruct {
    jpeg_source_mgr pub;                /* public fields */
    SvStream*   stream;                 /* source stream */
    JOCTET*     buffer;                 /* start of buffer */
    boolean     start_of_file;          /* have we gotten any data yet? */
};

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
extern "C" void init_source (j_decompress_ptr cinfo)
{
    SourceManagerStruct * source = (SourceManagerStruct *) cinfo->src;

    /* We reset the empty-input-file flag for each image,
     * but we don't clear the input buffer.
     * This is correct behavior for reading a series of images from one source.
     */
    source->start_of_file = sal_True;
}

long StreamRead( SvStream* pStream, void* pBuffer, long nBufferSize )
{
    long nRead = 0;

    if( pStream->GetError() != ERRCODE_IO_PENDING )
    {
        long nInitialPosition = pStream->Tell();

        nRead = (long) pStream->Read( pBuffer, nBufferSize );

        if( pStream->GetError() == ERRCODE_IO_PENDING )
        {
            
            
            pStream->ResetError();
            pStream->Seek( nInitialPosition );
            pStream->SetError( ERRCODE_IO_PENDING );
        }
    }

    return nRead;
}

extern "C" boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    SourceManagerStruct * source = (SourceManagerStruct *) cinfo->src;
    size_t nbytes;

    nbytes = StreamRead(source->stream, source->buffer, BUFFER_SIZE);

    if (!nbytes)
    {
        if (source->start_of_file)     /* Treat empty input file as fatal error */
        {
            ERREXIT(cinfo, JERR_INPUT_EMPTY);
        }
        WARNMS(cinfo, JWRN_JPEG_EOF);
        /* Insert a fake EOI marker */
        source->buffer[0] = (JOCTET) 0xFF;
        source->buffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
    }

    source->pub.next_input_byte = source->buffer;
    source->pub.bytes_in_buffer = nbytes;
    source->start_of_file = sal_False;

    return sal_True;
}

extern "C" void skip_input_data (j_decompress_ptr cinfo, long numberOfBytes)
{
    SourceManagerStruct * source = (SourceManagerStruct *) cinfo->src;

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (numberOfBytes > 0)
    {
        while (numberOfBytes > (long) source->pub.bytes_in_buffer)
        {
            numberOfBytes -= (long) source->pub.bytes_in_buffer;
            (void) fill_input_buffer(cinfo);

            /* note we assume that fill_input_buffer will never return sal_False,
             * so suspension need not be handled.
             */
        }
        source->pub.next_input_byte += (size_t) numberOfBytes;
        source->pub.bytes_in_buffer -= (size_t) numberOfBytes;
    }
}

extern "C" void term_source (j_decompress_ptr)
{
    /* no work necessary here */
}

void jpeg_svstream_src (j_decompress_ptr cinfo, void* input)
{
    SourceManagerStruct * source;
    SvStream* stream = (SvStream*)input;

    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     * This makes it unsafe to use this manager and a different source
     * manager serially with the same JPEG object.  Caveat programmer.
     */

    if (cinfo->src == NULL)
    { /* first time for this JPEG object? */
        cinfo->src = (jpeg_source_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(SourceManagerStruct));
        source = (SourceManagerStruct *) cinfo->src;
        source->buffer = (JOCTET *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, BUFFER_SIZE * sizeof(JOCTET));
    }

    source = (SourceManagerStruct *) cinfo->src;
    source->pub.init_source = init_source;
    source->pub.fill_input_buffer = fill_input_buffer;
    source->pub.skip_input_data = skip_input_data;
    source->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    source->pub.term_source = term_source;
    source->stream = stream;
    source->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    source->pub.next_input_byte = NULL; /* until buffer loaded */
}

JPEGReader::JPEGReader( SvStream& rStream, void* /*pCallData*/, bool bSetLogSize ) :
    mrStream         ( rStream ),
    mpAcc            ( NULL ),
    mpAcc1           ( NULL ),
    mpBuffer         ( NULL ),
    mnLastPos        ( rStream.Tell() ),
    mnLastLines      ( 0 ),
    mbSetLogSize     ( bSetLogSize )
{
    maUpperName = "SVIJPEG";
    mnFormerPos = mnLastPos;
}

JPEGReader::~JPEGReader()
{
    if( mpBuffer )
        delete[] mpBuffer;

    if( mpAcc )
        maBmp.ReleaseAccess( mpAcc );

    if( mpAcc1 )
        maBmp1.ReleaseAccess( mpAcc1 );
}

unsigned char * JPEGReader::CreateBitmap( JPEGCreateBitmapParam * pParam )
{
    if (pParam->nWidth > SAL_MAX_INT32 / 8 || pParam->nHeight > SAL_MAX_INT32 / 8)
        return NULL; 

    if (pParam->nWidth <= 0 || pParam->nHeight <=0)
        return NULL;

    Size        aSize( pParam->nWidth, pParam->nHeight );
    bool        bGray = pParam->bGray != 0;

    unsigned char * pBmpBuf = NULL;

    if( mpAcc )
    {
        maBmp.ReleaseAccess( mpAcc );
        maBmp = Bitmap();
        mpAcc = NULL;
    }

    sal_uInt64 nSize = aSize.Width();
    nSize *= aSize.Height();
    if (nSize > SAL_MAX_INT32 / (bGray?1:3))
        return NULL;

    
    if (nSize*(bGray?1:3) > MAX_BITMAP_BYTE_SIZE)
    {
        
        
        return NULL;
    }

    if( bGray )
    {
        BitmapPalette aGrayPal( 256 );

        for( sal_uInt16 n = 0; n < 256; n++ )
        {
            const sal_uInt8 cGray = (sal_uInt8) n;
            aGrayPal[ n ] = BitmapColor( cGray, cGray, cGray );
        }

        maBmp = Bitmap( aSize, 8, &aGrayPal );
    }
    else
    {
        maBmp = Bitmap( aSize, 24 );
    }

    if ( mbSetLogSize )
    {
        unsigned long nUnit = pParam->density_unit;

        if( ( ( 1 == nUnit ) || ( 2 == nUnit ) ) && pParam->X_density && pParam->Y_density )
        {
            Point       aEmptyPoint;
            Fraction    aFractX( 1, pParam->X_density );
            Fraction    aFractY( 1, pParam->Y_density );
            MapMode     aMapMode( nUnit == 1 ? MAP_INCH : MAP_CM, aEmptyPoint, aFractX, aFractY );
            Size        aPrefSize = OutputDevice::LogicToLogic( aSize, aMapMode, MAP_100TH_MM );

            maBmp.SetPrefSize( aPrefSize );
            maBmp.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
        }
    }

    mpAcc = maBmp.AcquireWriteAccess();

    if( mpAcc )
    {
        const sal_uLong nFormat = mpAcc->GetScanlineFormat();

        if(
            ( bGray && ( BMP_FORMAT_8BIT_PAL == nFormat ) ) ||
            ( !bGray && ( BMP_FORMAT_24BIT_TC_RGB == nFormat ) )
          )
        {
            pBmpBuf = mpAcc->GetBuffer();
            pParam->nAlignedWidth = mpAcc->GetScanlineSize();
            pParam->bTopDown = mpAcc->IsTopDown();
        }
        else
        {
            pParam->nAlignedWidth = AlignedWidth4Bytes( aSize.Width() * ( bGray ? 8 : 24 ) );
            pParam->bTopDown = true;
            pBmpBuf = mpBuffer = new unsigned char[pParam->nAlignedWidth * aSize.Height()];
        }
    }

    
    if ( !pBmpBuf )
    {
        maBmp.ReleaseAccess( mpAcc );
        maBmp = Bitmap();
        mpAcc = NULL;
    }

    return pBmpBuf;
}

void JPEGReader::FillBitmap()
{
    if( mpBuffer && mpAcc )
    {
        unsigned char * pTmp;
        BitmapColor aColor;
        long        nAlignedWidth;
        long        nWidth = mpAcc->Width();
        long        nHeight = mpAcc->Height();

        if( mpAcc->GetBitCount() == 8 )
        {
            BitmapColor* pCols = new BitmapColor[ 256 ];

            for( sal_uInt16 n = 0; n < 256; n++ )
            {
                const sal_uInt8 cGray = (sal_uInt8) n;
                pCols[ n ] = mpAcc->GetBestMatchingColor( BitmapColor( cGray, cGray, cGray ) );
            }

            nAlignedWidth = AlignedWidth4Bytes( mpAcc->Width() * 8L );

            for( long nY = 0L; nY < nHeight; nY++ )
            {
                pTmp = (sal_uInt8*) mpBuffer + nY * nAlignedWidth;

                for( long nX = 0L; nX < nWidth; nX++ )
                {
                    mpAcc->SetPixel( nY, nX, pCols[ *pTmp++ ] );
                }
            }

            delete[] pCols;
        }
        else
        {
            nAlignedWidth = AlignedWidth4Bytes( mpAcc->Width() * 24L );

            for( long nY = 0L; nY < nHeight; nY++ )
            {
                
                static bool bCheckOwnReader(true);

                if(bCheckOwnReader)
                {
                    
                    
                    
                    
                    
                    sal_uInt8* aSource((sal_uInt8*)mpBuffer + nY * nAlignedWidth);
                    sal_uInt8* aEnd(aSource + (nWidth * 3));

                    for(sal_uInt8* aTmp(aSource); aTmp < aEnd; aTmp += 3)
                    {
                        ::std::swap(*aTmp, *(aTmp + 2));
                    }

                    mpAcc->CopyScanline(nY, aSource, BMP_FORMAT_24BIT_TC_BGR, nWidth * 3);
                }
                else
                {
                    
                    pTmp = (sal_uInt8*) mpBuffer + nY * nAlignedWidth;

                    for( long nX = 0L; nX < nWidth; nX++ )
                    {
                        aColor.SetRed( *pTmp++ );
                        aColor.SetGreen( *pTmp++ );
                        aColor.SetBlue( *pTmp++ );
                        mpAcc->SetPixel( nY, nX, aColor );
                    }
                }
            }
        }
    }
}

Graphic JPEGReader::CreateIntermediateGraphic( const Bitmap& rBitmap, long nLines )
{
    Graphic     aGraphic;
    const Size  aSizePixel( rBitmap.GetSizePixel() );

    if( !mnLastLines )
    {
        if( mpAcc1 )
        {
            maBmp1.ReleaseAccess( mpAcc1 );
        }

        maBmp1 = Bitmap( rBitmap.GetSizePixel(), 1 );
        maBmp1.Erase( Color( COL_WHITE ) );
        mpAcc1 = maBmp1.AcquireWriteAccess();
    }

    if( nLines && ( nLines < aSizePixel.Height() ) )
    {
        if( mpAcc1 )
        {
            const long nNewLines = nLines - mnLastLines;

            if( nNewLines )
            {
                mpAcc1->SetFillColor( Color( COL_BLACK ) );
                mpAcc1->FillRect( Rectangle( Point( 0, mnLastLines ), Size( mpAcc1->Width(), nNewLines ) ) );
            }

            maBmp1.ReleaseAccess( mpAcc1 );
            aGraphic = BitmapEx( rBitmap, maBmp1 );
            mpAcc1 = maBmp1.AcquireWriteAccess();
        }
        else
        {
            aGraphic = rBitmap;
        }
    }
    else
        aGraphic = rBitmap;

    mnLastLines = nLines;

    return aGraphic;
}

ReadState JPEGReader::Read( Graphic& rGraphic )
{
    long        nEndPosition;
    long        nLines;
    ReadState   eReadState;
    bool        bRet = false;
    sal_uInt8   cDummy;

    
    
    mrStream.Seek( STREAM_SEEK_TO_END );
    mrStream.ReadUChar( cDummy );
    nEndPosition = mrStream.Tell();

    
    if( mrStream.GetError() == ERRCODE_IO_PENDING )
    {
        mrStream.ResetError();
        if( ( nEndPosition  - mnFormerPos ) < JPEG_MIN_READ )
        {
            mrStream.Seek( mnLastPos );
            return JPEGREAD_NEED_MORE;
        }
    }

    
    mrStream.Seek( mnLastPos );

    
    ReadJPEG( this, &mrStream, &nLines, GetPreviewSize() );

    if( mpAcc )
    {
        if( mpBuffer )
        {
            FillBitmap();
            delete[] mpBuffer;
            mpBuffer = NULL;
        }

        maBmp.ReleaseAccess( mpAcc );
        mpAcc = NULL;

        if( mrStream.GetError() == ERRCODE_IO_PENDING )
        {
            rGraphic = CreateIntermediateGraphic( maBmp, nLines );
        }
        else
        {
            rGraphic = maBmp;
        }

        bRet = true;
    }
    else if( mrStream.GetError() == ERRCODE_IO_PENDING )
    {
        bRet = true;
    }

    
    if( mrStream.GetError() == ERRCODE_IO_PENDING )
    {
        eReadState = JPEGREAD_NEED_MORE;
        mrStream.ResetError();
        mnFormerPos = mrStream.Tell();
    }
    else
    {
        eReadState = bRet ? JPEGREAD_OK : JPEGREAD_ERROR;
    }

    return eReadState;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
