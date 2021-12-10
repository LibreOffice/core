/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include "jpeg.h"
#include <jpeglib.h>
#include <jerror.h>

#include "JpegReader.hxx"
#include <vcl/graphicfilter.hxx>
#include <vcl/outdev.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <memory>

#define BUFFER_SIZE  4096

extern "C" {

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void init_source (j_decompress_ptr cinfo)
{
    SourceManagerStruct * source = reinterpret_cast<SourceManagerStruct *>(cinfo->src);

    /* We reset the empty-input-file flag for each image,
     * but we don't clear the input buffer.
     * This is correct behavior for reading a series of images from one source.
     */
    source->start_of_file = TRUE;
    source->no_data_available_failures = 0;
}

}

static tools::Long StreamRead( SvStream* pStream, void* pBuffer, tools::Long nBufferSize )
{
    tools::Long nRead = 0;

    if( pStream->GetError() != ERRCODE_IO_PENDING )
    {
        sal_uInt64 nInitialPosition = pStream->Tell();

        nRead = static_cast<tools::Long>(pStream->ReadBytes(pBuffer, nBufferSize));

        if( pStream->GetError() == ERRCODE_IO_PENDING )
        {
            // in order to search from the old position
            // we temporarily reset the error
            pStream->ResetError();
            pStream->Seek( nInitialPosition );
            pStream->SetError( ERRCODE_IO_PENDING );
        }
    }

    return nRead;
}

extern "C" {

static boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    SourceManagerStruct * source = reinterpret_cast<SourceManagerStruct *>(cinfo->src);
    size_t nbytes;

    nbytes = StreamRead(source->stream, source->buffer, BUFFER_SIZE);

    if (!nbytes)
    {
        source->no_data_available_failures++;
        if (source->start_of_file)     /* Treat empty input file as fatal error */
        {
            ERREXIT(cinfo, JERR_INPUT_EMPTY);
        }
        WARNMS(cinfo, JWRN_JPEG_EOF);
        /* Insert a fake EOI marker */
        source->buffer[0] = JOCTET(0xFF);
        source->buffer[1] = JOCTET(JPEG_EOI);
        nbytes = 2;
    }

    source->pub.next_input_byte = source->buffer;
    source->pub.bytes_in_buffer = nbytes;
    source->start_of_file = FALSE;

    return TRUE;
}

static void skip_input_data (j_decompress_ptr cinfo, long numberOfBytes)
{
    SourceManagerStruct * source = reinterpret_cast<SourceManagerStruct *>(cinfo->src);

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (numberOfBytes <= 0)
        return;

    while (numberOfBytes > static_cast<tools::Long>(source->pub.bytes_in_buffer))
    {
        numberOfBytes -= static_cast<tools::Long>(source->pub.bytes_in_buffer);
        (void) fill_input_buffer(cinfo);

        /* note we assume that fill_input_buffer will never return false,
         * so suspension need not be handled.
         */
    }
    source->pub.next_input_byte += static_cast<size_t>(numberOfBytes);
    source->pub.bytes_in_buffer -= static_cast<size_t>(numberOfBytes);
}

static void term_source (j_decompress_ptr)
{
    /* no work necessary here */
}

}

void jpeg_svstream_src (j_decompress_ptr cinfo, void* input)
{
    SourceManagerStruct * source;
    SvStream* stream = static_cast<SvStream*>(input);

    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     * This makes it unsafe to use this manager and a different source
     * manager serially with the same JPEG object.  Caveat programmer.
     */

    if (cinfo->src == nullptr)
    { /* first time for this JPEG object? */
        cinfo->src = static_cast<jpeg_source_mgr *>(
            (*cinfo->mem->alloc_small) (reinterpret_cast<j_common_ptr>(cinfo), JPOOL_PERMANENT, sizeof(SourceManagerStruct)));
        source = reinterpret_cast<SourceManagerStruct *>(cinfo->src);
        source->buffer = static_cast<JOCTET *>(
            (*cinfo->mem->alloc_small) (reinterpret_cast<j_common_ptr>(cinfo), JPOOL_PERMANENT, BUFFER_SIZE * sizeof(JOCTET)));
    }

    source = reinterpret_cast<SourceManagerStruct*>(cinfo->src);
    source->pub.init_source = init_source;
    source->pub.fill_input_buffer = fill_input_buffer;
    source->pub.skip_input_data = skip_input_data;
    source->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    source->pub.term_source = term_source;
    source->stream = stream;
    source->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    source->pub.next_input_byte = nullptr; /* until buffer loaded */
}

JPEGReader::JPEGReader( SvStream& rStream, GraphicFilterImportFlags nImportFlags ) :
    mrStream         ( rStream ),
    mnLastPos        ( rStream.Tell() ),
    mnLastLines      ( 0 ),
    mbSetLogSize     ( nImportFlags & GraphicFilterImportFlags::SetLogsizeForJpeg )
{
    maUpperName = "SVIJPEG";

    if (!(nImportFlags & GraphicFilterImportFlags::UseExistingBitmap))
    {
        mpBitmap.reset(new Bitmap());
        mpIncompleteAlpha.reset(new Bitmap());
    }
}

JPEGReader::~JPEGReader()
{
}

bool JPEGReader::CreateBitmap(JPEGCreateBitmapParam const & rParam)
{
    if (rParam.nWidth > SAL_MAX_INT32 / 8 || rParam.nHeight > SAL_MAX_INT32 / 8)
        return false; // avoid overflows later

    if (rParam.nWidth == 0 || rParam.nHeight == 0)
        return false;

    Size aSize(rParam.nWidth, rParam.nHeight);
    bool bGray = rParam.bGray;

    mpBitmap.reset(new Bitmap());

    sal_uInt64 nSize = aSize.Width() * aSize.Height();

    if (nSize > SAL_MAX_INT32 / (bGray?1:3))
        return false;

    if( bGray )
    {
        BitmapPalette aGrayPal( 256 );

        for( sal_uInt16 n = 0; n < 256; n++ )
        {
            const sal_uInt8 cGray = static_cast<sal_uInt8>(n);
            aGrayPal[ n ] = BitmapColor( cGray, cGray, cGray );
        }

        mpBitmap.reset(new Bitmap(aSize, vcl::PixelFormat::N8_BPP, &aGrayPal));
    }
    else
    {
        mpBitmap.reset(new Bitmap(aSize, vcl::PixelFormat::N24_BPP));
    }

    if (mbSetLogSize)
    {
        unsigned long nUnit = rParam.density_unit;

        if (((1 == nUnit) || (2 == nUnit)) && rParam.X_density && rParam.Y_density )
        {
            Fraction    aFractX( 1, rParam.X_density );
            Fraction    aFractY( 1, rParam.Y_density );
            MapMode     aMapMode( nUnit == 1 ? MapUnit::MapInch : MapUnit::MapCM, Point(), aFractX, aFractY );
            Size        aPrefSize = OutputDevice::LogicToLogic(aSize, aMapMode, MapMode(MapUnit::Map100thMM));

            mpBitmap->SetPrefSize(aPrefSize);
            mpBitmap->SetPrefMapMode(MapMode(MapUnit::Map100thMM));
        }
    }

    return true;
}

Graphic JPEGReader::CreateIntermediateGraphic(tools::Long nLines)
{
    Graphic aGraphic;
    const Size aSizePixel(mpBitmap->GetSizePixel());

    if (!mnLastLines)
    {
        mpIncompleteAlpha.reset(new Bitmap(aSizePixel, vcl::PixelFormat::N1_BPP));
        mpIncompleteAlpha->Erase(COL_WHITE);
    }

    if (nLines && (nLines < aSizePixel.Height()))
    {
        const tools::Long nNewLines = nLines - mnLastLines;

        if (nNewLines > 0)
        {
            {
                BitmapScopedWriteAccess pAccess(*mpIncompleteAlpha);
                pAccess->SetFillColor(COL_BLACK);
                pAccess->FillRect(tools::Rectangle(Point(0, mnLastLines), Size(pAccess->Width(), nNewLines)));
            }

            aGraphic = BitmapEx(*mpBitmap, *mpIncompleteAlpha);
        }
        else
        {
            aGraphic = BitmapEx(*mpBitmap);
        }
    }
    else
    {
        aGraphic = BitmapEx(*mpBitmap);
    }

    mnLastLines = nLines;

    return aGraphic;
}

ReadState JPEGReader::Read( Graphic& rGraphic, GraphicFilterImportFlags nImportFlags, BitmapScopedWriteAccess* ppAccess )
{
    ReadState   eReadState;
    bool        bRet = false;

    // seek back to the original position
    mrStream.Seek( mnLastPos );

    // read the (partial) image
    tools::Long nLines;
    ReadJPEG( this, &mrStream, &nLines, nImportFlags, ppAccess );

    auto bUseExistingBitmap = static_cast<bool>(nImportFlags & GraphicFilterImportFlags::UseExistingBitmap);
    if (bUseExistingBitmap || !mpBitmap->IsEmpty())
    {
        if( mrStream.GetError() == ERRCODE_IO_PENDING )
        {
            rGraphic = CreateIntermediateGraphic(nLines);
        }
        else
        {
            if (!bUseExistingBitmap)
                rGraphic = BitmapEx(*mpBitmap);
        }

        bRet = true;
    }
    else if( mrStream.GetError() == ERRCODE_IO_PENDING )
    {
        bRet = true;
    }

    // Set status ( Pending has priority )
    if (mrStream.GetError() == ERRCODE_IO_PENDING)
    {
        eReadState = JPEGREAD_NEED_MORE;
        mrStream.ResetError();
    }
    else
    {
        eReadState = bRet ? JPEGREAD_OK : JPEGREAD_ERROR;
    }

    return eReadState;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
