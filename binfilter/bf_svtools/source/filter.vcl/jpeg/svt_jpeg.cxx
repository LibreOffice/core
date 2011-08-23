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

// MARKER(update_precomp.py): autogen include statement, do not remove

#include <tools/solar.h>

extern "C"
{
    #define INT32 JPEG_INT32
    #include "stdio.h"
    #include "jpeg.h"
    #include "jpeglib.h"
    #include "jerror.h"
    #undef INT32
}

#define _JPEGPRIVATE
#include <vcl/bmpacc.hxx>
#include "jpeg.hxx"
#include <bf_svtools/FilterConfigItem.hxx>
#include <bf_svtools/filter.hxx>

// -----------
// - Defines -
// -----------

using namespace ::com::sun::star;

namespace binfilter
{

#define JPEGMINREAD 512

// -------------
// - (C-Calls) -
// -------------

// ------------------------------------------------------------------------

extern "C" void* CreateBitmap( void* pJPEGReader, void* pJPEGCreateBitmapParam )
{
    return ( (JPEGReader*) pJPEGReader )->CreateBitmap( pJPEGCreateBitmapParam );
}

// ------------------------------------------------------------------------

extern "C" void* GetScanline( void* pJPEGWriter, long nY )
{
    return ( (JPEGWriter*) pJPEGWriter )->GetScanline( nY );
}

// ------------------------------------------------------------------------

struct JPEGCallbackStruct
{
    uno::Reference< task::XStatusIndicator > xStatusIndicator;
};

extern "C" long JPEGCallback( void* pCallbackData, long nPercent )
{
    JPEGCallbackStruct* pS = (JPEGCallbackStruct*)pCallbackData;
    if ( pS && pS->xStatusIndicator.is() )
    {
        pS->xStatusIndicator->setValue( nPercent );
    }
    return 0L;
}

#define BUF_SIZE  4096

typedef struct 
{
  struct jpeg_destination_mgr pub;  /* public fields */

  SvStream* outfile;                /* target stream */
  JOCTET * buffer;                  /* start of buffer */
} my_destination_mgr;

typedef my_destination_mgr * my_dest_ptr;

extern "C" void init_destination (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

  /* Allocate the output buffer --- it will be released when done with image */
  dest->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                                  BUF_SIZE * sizeof(JOCTET));

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = BUF_SIZE;
}

extern "C" int empty_output_buffer (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

  if (dest->outfile->Write(dest->buffer, BUF_SIZE) !=
      (size_t) BUF_SIZE)
    ERREXIT(cinfo, JERR_FILE_WRITE);

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = BUF_SIZE;

  return TRUE;
}

extern "C" void term_destination (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
  size_t datacount = BUF_SIZE - dest->pub.free_in_buffer;

  /* Write any data remaining in the buffer */
  if (datacount > 0) {
    if (dest->outfile->Write(dest->buffer, datacount) != datacount)
      ERREXIT(cinfo, JERR_FILE_WRITE);
  }
}

extern "C" void jpeg_svstream_dest (j_compress_ptr cinfo, void* out)
{
  SvStream * outfile = (SvStream*)out;
  my_dest_ptr dest;

  /* The destination object is made permanent so that multiple JPEG images
   * can be written to the same file without re-executing jpeg_svstream_dest.
   * This makes it dangerous to use this manager and a different destination
   * manager serially with the same JPEG object, because their private object
   * sizes may be different.  Caveat programmer.
   */
  if (cinfo->dest == NULL) {    /* first time for this JPEG object? */
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  sizeof(my_destination_mgr));
  }

  dest = (my_dest_ptr) cinfo->dest;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->outfile = outfile;
}

/* Expanded data source object for stdio input */

typedef struct {
  struct jpeg_source_mgr pub;   /* public fields */

  SvStream * infile;            /* source stream */
  JOCTET * buffer;              /* start of buffer */
  boolean start_of_file;        /* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

extern "C" void init_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}

long StreamRead( SvStream* pSvStm, void* pBuffer, long nBufferSize )
{
        long            nRead;

        if( pSvStm->GetError() != ERRCODE_IO_PENDING )
        {
                long nActPos = pSvStm->Tell();

                nRead = (long) pSvStm->Read( pBuffer, nBufferSize );

                if( pSvStm->GetError() == ERRCODE_IO_PENDING )
                {
                        nRead = 0;

                        // Damit wir wieder an die alte Position
                        // seeken koennen, setzen wir den Error temp.zurueck
                        pSvStm->ResetError();
                        pSvStm->Seek( nActPos );
                        pSvStm->SetError( ERRCODE_IO_PENDING );
                }
        }
        else
                nRead = 0;

        return nRead;
}

extern "C" int fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;
  size_t nbytes;

  nbytes = StreamRead(src->infile, src->buffer, BUF_SIZE);

  if (nbytes <= 0) {
    if (src->start_of_file)     /* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

  return TRUE;
}

extern "C" void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}

extern "C" void term_source (j_decompress_ptr)
{
  /* no work necessary here */
}

extern "C" void jpeg_svstream_src (j_decompress_ptr cinfo, void * in)
{
  my_src_ptr src;
  SvStream * infile = (SvStream*)in;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) {     /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  sizeof(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  BUF_SIZE * sizeof(JOCTET));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->infile = infile;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}

// --------------
// - JPEGReader -
// --------------

JPEGReader::JPEGReader( SvStream& rStm, void* /*pCallData*/, sal_Bool bSetLS ) :
        rIStm			( rStm ),
        pAcc			( NULL ),
        pAcc1			( NULL ),
        pBuffer			( NULL ),
        nLastPos		( rStm.Tell() ),
        nLastLines		( 0 ),
        bSetLogSize     ( bSetLS )
{
    maUpperName = String::CreateFromAscii( "SVIJPEG", 7 );
    nFormerPos = nLastPos;
}

// ------------------------------------------------------------------------

JPEGReader::~JPEGReader()
{
    if( pBuffer )
        rtl_freeMemory( pBuffer );

    if( pAcc )
        aBmp.ReleaseAccess( pAcc );

    if( pAcc1 )
        aBmp1.ReleaseAccess( pAcc1 );
}

// ------------------------------------------------------------------------

void* JPEGReader::CreateBitmap( void* pParam )
{
    Size        aSize( ((JPEGCreateBitmapParam*)pParam)->nWidth,
                        ((JPEGCreateBitmapParam*)pParam)->nHeight );
    sal_Bool    bGray = ((JPEGCreateBitmapParam*)pParam)->bGray != 0;

    void* pBmpBuf = NULL;

    if( pAcc )
        aBmp.ReleaseAccess( pAcc );

    if( bGray )
    {
        BitmapPalette aGrayPal( 256 );

        for( USHORT n = 0; n < 256; n++ )
        {
            const BYTE cGray = (BYTE) n;
            aGrayPal[ n ] = BitmapColor( cGray, cGray, cGray );
        }

        aBmp = Bitmap( aSize, 8, &aGrayPal );
    }
    else
        aBmp = Bitmap( aSize, 24 );
   
    if ( bSetLogSize )
    {
        unsigned long nUnit = ((JPEGCreateBitmapParam*)pParam)->density_unit;
        
        if( ( ( 1 == nUnit ) || ( 2 == nUnit ) ) && 
            ( (JPEGCreateBitmapParam*) pParam )->X_density && 
            ( (JPEGCreateBitmapParam*) pParam )->Y_density )
        {
            Point       aEmptyPoint;
            Fraction	aFractX( 1, ((JPEGCreateBitmapParam*)pParam)->X_density );
            Fraction	aFractY( 1, ((JPEGCreateBitmapParam*)pParam)->Y_density );
            MapMode		aMapMode( nUnit == 1 ? MAP_INCH : MAP_CM, aEmptyPoint, aFractX, aFractY );
            Size		aPrefSize = OutputDevice::LogicToLogic( aSize, aMapMode, MAP_100TH_MM );

            aBmp.SetPrefSize( aPrefSize );
            aBmp.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
        }
    }
    
    pAcc = aBmp.AcquireWriteAccess();

    if( pAcc )
    {
        long nAlignedWidth;

        const ULONG nFormat = pAcc->GetScanlineFormat();

        if(
            ( bGray && ( BMP_FORMAT_8BIT_PAL == nFormat ) ) ||
            ( !bGray && ( BMP_FORMAT_24BIT_TC_RGB == nFormat ) )
          )
        {
            pBmpBuf = pAcc->GetBuffer();
            nAlignedWidth = pAcc->GetScanlineSize();
            ((JPEGCreateBitmapParam*)pParam)->bTopDown = pAcc->IsTopDown();
        }
        else
        {
            nAlignedWidth = AlignedWidth4Bytes( aSize.Width() * ( bGray ? 8 : 24 ) );
            ((JPEGCreateBitmapParam*)pParam)->bTopDown = TRUE;
            pBmpBuf = pBuffer = rtl_allocateMemory( nAlignedWidth * aSize.Height() );
        }
        ((JPEGCreateBitmapParam*)pParam)->nAlignedWidth = nAlignedWidth;
    }

    return pBmpBuf;
}

// ------------------------------------------------------------------------

void JPEGReader::FillBitmap()
{
    if( pBuffer && pAcc )
    {
        HPBYTE		pTmp;
        BitmapColor	aColor;
        long		nAlignedWidth;
        long		nWidth = pAcc->Width();
        long		nHeight = pAcc->Height();

        if( pAcc->GetBitCount() == 8 )
        {
            BitmapColor* pCols = new BitmapColor[ 256 ];

            for( USHORT n = 0; n < 256; n++ )
            {
                const BYTE cGray = (BYTE) n;
                pCols[ n ] = pAcc->GetBestMatchingColor( BitmapColor( cGray, cGray, cGray ) );
            }

            nAlignedWidth = AlignedWidth4Bytes( pAcc->Width() * 8L );

            for( long nY = 0L; nY < nHeight; nY++ )
            {
                pTmp = (BYTE*) pBuffer + nY * nAlignedWidth;

                for( long nX = 0L; nX < nWidth; nX++ )
                    pAcc->SetPixel( nY, nX, pCols[ *pTmp++ ] );
            }

            delete[] pCols;
        }
        else
        {
            nAlignedWidth = AlignedWidth4Bytes( pAcc->Width() * 24L );

            for( long nY = 0L; nY < nHeight; nY++ )
            {
                pTmp = (BYTE*) pBuffer + nY * nAlignedWidth;

                for( long nX = 0L; nX < nWidth; nX++ )
                {
                    aColor.SetRed( *pTmp++ );
                    aColor.SetGreen( *pTmp++ );
                    aColor.SetBlue( *pTmp++ );
                    pAcc->SetPixel( nY, nX, aColor );
                }
            }
        }
    }
}

// ------------------------------------------------------------------------

Graphic JPEGReader::CreateIntermediateGraphic( const Bitmap& rBitmap, long nLines )
{
    Graphic		aGraphic;
    const Size	aSizePix( rBitmap.GetSizePixel() );

    if( !nLastLines )
    {
        if( pAcc1 )
            aBmp1.ReleaseAccess( pAcc1 );

        aBmp1 = Bitmap( rBitmap.GetSizePixel(), 1 );
        aBmp1.Erase( Color( COL_WHITE ) );
        pAcc1 = aBmp1.AcquireWriteAccess();
    }

    if( nLines && ( nLines < aSizePix.Height() ) )
    {
        if( pAcc1 )
        {
            const long nNewLines = nLines - nLastLines;

            if( nNewLines )
            {
                pAcc1->SetFillColor( Color( COL_BLACK ) );
                pAcc1->FillRect( Rectangle( Point( 0, nLastLines ),
                                            Size( pAcc1->Width(), nNewLines ) ) );
            }

            aBmp1.ReleaseAccess( pAcc1 );
            aGraphic = BitmapEx( rBitmap, aBmp1 );
            pAcc1 = aBmp1.AcquireWriteAccess();
        }
        else
            aGraphic = rBitmap;
    }
    else
        aGraphic = rBitmap;

    nLastLines = nLines;

    return aGraphic;
}

// ------------------------------------------------------------------------

ReadState JPEGReader::Read( Graphic& rGraphic )
{
    long		nEndPos;
    long		nLines;
    ReadState	eReadState;
    BOOL		bRet = FALSE;
    BYTE		cDummy;

#if 1 // TODO: is it possible to get rid of this seek to the end?
    // check if the stream's end is already available
    rIStm.Seek( STREAM_SEEK_TO_END );
    rIStm >> cDummy;
    nEndPos = rIStm.Tell();

    // else check if at least JPEGMINREAD bytes can be read
    if( rIStm.GetError() == ERRCODE_IO_PENDING )
    {
        rIStm.ResetError();
        if( ( nEndPos  - nFormerPos ) < JPEGMINREAD )
        {
            rIStm.Seek( nLastPos );
            return JPEGREAD_NEED_MORE;
        }
    }

    // seek back to the original position
    rIStm.Seek( nLastPos );
#endif

    Size aPreviewSize = GetPreviewSize();
    SetJpegPreviewSizeHint( aPreviewSize.Width(), aPreviewSize.Height() );

    // read the (partial) image
    ReadJPEG( this, &rIStm, &nLines );

    if( pAcc )
    {
        if( pBuffer )
        {
            FillBitmap();
            rtl_freeMemory( pBuffer );
            pBuffer = NULL;
        }

        aBmp.ReleaseAccess( pAcc );
        pAcc = NULL;

        if( rIStm.GetError() == ERRCODE_IO_PENDING )
            rGraphic = CreateIntermediateGraphic( aBmp, nLines );
        else
            rGraphic = aBmp;

        bRet = TRUE;
    }
    else if( rIStm.GetError() == ERRCODE_IO_PENDING )
        bRet = TRUE;

    // Status setzen ( Pending hat immer Vorrang )
    if( rIStm.GetError() == ERRCODE_IO_PENDING )
    {
        eReadState = JPEGREAD_NEED_MORE;
        rIStm.ResetError();
        nFormerPos = rIStm.Tell();
    }
    else
    {
        if( bRet )
            eReadState = JPEGREAD_OK;
        else
            eReadState = JPEGREAD_ERROR;
    }

    return eReadState;
}


// --------------
// - JPEGWriter -
// --------------

JPEGWriter::JPEGWriter( SvStream& rStm, const uno::Sequence< beans::PropertyValue >* pFilterData ) :
        rOStm		( rStm ),
        pAcc		( NULL ),
        pBuffer		( NULL )
{
    FilterConfigItem aConfigItem( (uno::Sequence< beans::PropertyValue >*)pFilterData );
    bGreys = aConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "ColorMode" ) ), 0 ) != 0;
    nQuality = aConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), 75 );

    if ( pFilterData )
    {
        int nArgs = pFilterData->getLength();
        const beans::PropertyValue* pValues = pFilterData->getConstArray();
        while( nArgs-- )
        {
            if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StatusIndicator" ) ) )
            {
                pValues->Value >>= xStatusIndicator;
            }
            pValues++;
        }
    }
}

// ------------------------------------------------------------------------

void* JPEGWriter::GetScanline( long nY )
{
    void* pScanline = NULL;

    if( pAcc )
    {
        if( bNative )
            pScanline = pAcc->GetScanline( nY );
        else if( pBuffer )
        {
            BitmapColor aColor;
            long		nWidth = pAcc->Width();
            BYTE*		pTmp = pBuffer;

            if( pAcc->HasPalette() )
            {
                for( long nX = 0L; nX < nWidth; nX++ )
                {
                    aColor = pAcc->GetPaletteColor( (BYTE) pAcc->GetPixel( nY, nX ) );
                    *pTmp++ = aColor.GetRed();
                    *pTmp++ = aColor.GetGreen();
                    *pTmp++ = aColor.GetBlue();
                }
            }
            else
            {
                for( long nX = 0L; nX < nWidth; nX++ )
                {
                    aColor = pAcc->GetPixel( nY, nX );
                    *pTmp++ = aColor.GetRed();
                    *pTmp++ = aColor.GetGreen();
                    *pTmp++ = aColor.GetBlue();
                }
            }

            pScanline = pBuffer;
        }
    }

    return pScanline;
}

// ------------------------------------------------------------------------

BOOL JPEGWriter::Write( const Graphic& rGraphic )
{
    BOOL bRet = FALSE;

    if ( xStatusIndicator.is() )
    {
        rtl::OUString aMsg;
        xStatusIndicator->start( aMsg, 100 );
    }

    Bitmap aGraphicBmp( rGraphic.GetBitmap() );

    if ( bGreys )
    {
        if ( !aGraphicBmp.Convert( BMP_CONVERSION_8BIT_GREYS ) )
            aGraphicBmp = rGraphic.GetBitmap();
    }

    pAcc = aGraphicBmp.AcquireReadAccess();

    if( pAcc )
    {
        bNative = ( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB );

        if( !bNative )
            pBuffer = new BYTE[ AlignedWidth4Bytes( pAcc->Width() * 24L ) ];

        JPEGCallbackStruct aCallbackData;
        aCallbackData.xStatusIndicator = xStatusIndicator;
        bRet = (BOOL) WriteJPEG( this, &rOStm, pAcc->Width(), pAcc->Height(), nQuality, &aCallbackData );

        delete[] pBuffer;
        pBuffer = NULL;

        aGraphicBmp.ReleaseAccess( pAcc );
        pAcc = NULL;
    }
    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return bRet;
}

// --------------
// - ImportJPEG -
// --------------

BOOL ImportJPEG( SvStream& rStm, Graphic& rGraphic, void* pCallerData, sal_Int32 nImportFlags )
{
    JPEGReader*	pJPEGReader = (JPEGReader*) rGraphic.GetContext();
    ReadState	eReadState;
    BOOL		bRet = TRUE;

    if( !pJPEGReader )
        pJPEGReader = new JPEGReader( rStm, pCallerData, ( nImportFlags & GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG ) != 0 );

    if( nImportFlags & GRFILTER_I_FLAGS_FOR_PREVIEW )
        pJPEGReader->SetPreviewSize( Size(128,128) );
    else
        pJPEGReader->DisablePreviewMode();

    rGraphic.SetContext( NULL );
    eReadState = pJPEGReader->Read( rGraphic );

    if( eReadState == JPEGREAD_ERROR )
    {
        bRet = FALSE;
        delete pJPEGReader;
    }
    else if( eReadState == JPEGREAD_OK )
        delete pJPEGReader;
    else
        rGraphic.SetContext( pJPEGReader );

    return bRet;
}

//  --------------
//  - ExportJPEG -
//  --------------

BOOL ExportJPEG( SvStream& rOStm, const Graphic& rGraphic, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData )
{
    JPEGWriter aJPEGWriter( rOStm, pFilterData );
    return aJPEGWriter.Write( rGraphic );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
