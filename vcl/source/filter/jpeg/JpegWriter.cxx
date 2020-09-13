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
#include <sal/log.hxx>

#include "jpeg.h"
#include <jpeglib.h>
#include <jerror.h>

#include "JpegWriter.hxx"
#include <vcl/bitmapaccess.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>

#define BUFFER_SIZE  4096

namespace {

struct DestinationManagerStruct
{
    jpeg_destination_mgr pub;         /* public fields */
    SvStream* stream;                 /* target stream */
    JOCTET * buffer;                  /* start of buffer */
};

}

extern "C" {

static void init_destination (j_compress_ptr cinfo)
{
    DestinationManagerStruct * destination = reinterpret_cast<DestinationManagerStruct *>(cinfo->dest);

    /* Allocate the output buffer -- it will be released when done with image */
    destination->buffer = static_cast<JOCTET *>(
        (*cinfo->mem->alloc_small) (reinterpret_cast<j_common_ptr>(cinfo), JPOOL_IMAGE, BUFFER_SIZE * sizeof(JOCTET)));

    destination->pub.next_output_byte = destination->buffer;
    destination->pub.free_in_buffer = BUFFER_SIZE;
}

static boolean empty_output_buffer (j_compress_ptr cinfo)
{
    DestinationManagerStruct * destination = reinterpret_cast<DestinationManagerStruct *>(cinfo->dest);

    if (destination->stream->WriteBytes(destination->buffer, BUFFER_SIZE) != BUFFER_SIZE)
    {
        ERREXIT(cinfo, JERR_FILE_WRITE);
    }

    destination->pub.next_output_byte = destination->buffer;
    destination->pub.free_in_buffer = BUFFER_SIZE;

    return TRUE;
}

static void term_destination (j_compress_ptr cinfo)
{
    DestinationManagerStruct * destination = reinterpret_cast<DestinationManagerStruct *>(cinfo->dest);
    size_t datacount = BUFFER_SIZE - destination->pub.free_in_buffer;

    /* Write any data remaining in the buffer */
    if (datacount > 0)
    {
        if (destination->stream->WriteBytes(destination->buffer, datacount) != datacount)
        {
            ERREXIT(cinfo, JERR_FILE_WRITE);
        }
    }
}

}

void jpeg_svstream_dest (j_compress_ptr cinfo, void* output)
{
    SvStream* stream = static_cast<SvStream*>(output);
    DestinationManagerStruct * destination;

    /* The destination object is made permanent so that multiple JPEG images
     * can be written to the same file without re-executing jpeg_svstream_dest.
     * This makes it dangerous to use this manager and a different destination
     * manager serially with the same JPEG object, because their private object
     * sizes may be different.  Caveat programmer.
     */
    if (cinfo->dest == nullptr)
    {    /* first time for this JPEG object? */
        cinfo->dest = static_cast<jpeg_destination_mgr*>(
        (*cinfo->mem->alloc_small) (reinterpret_cast<j_common_ptr>(cinfo), JPOOL_PERMANENT, sizeof(DestinationManagerStruct)));
    }

    destination = reinterpret_cast<DestinationManagerStruct *>(cinfo->dest);
    destination->pub.init_destination = init_destination;
    destination->pub.empty_output_buffer = empty_output_buffer;
    destination->pub.term_destination = term_destination;
    destination->stream = stream;
}

JPEGWriter::JPEGWriter( SvStream& rStream, const css::uno::Sequence< css::beans::PropertyValue >* pFilterData, bool* pExportWasGrey ) :
    mrStream     ( rStream ),
    mpBuffer     ( nullptr ),
    mbNative     ( false ),
    mpExpWasGrey ( pExportWasGrey )
{
    FilterConfigItem aConfigItem( pFilterData );
    mbGreys = aConfigItem.ReadInt32( "ColorMode", 0 ) != 0;
    mnQuality = aConfigItem.ReadInt32( "Quality", 75 );
    maChromaSubsampling = aConfigItem.ReadInt32( "ChromaSubsamplingMode", 0 );

    if ( pFilterData )
    {
        for( const auto& rValue : *pFilterData )
        {
            if ( rValue.Name == "StatusIndicator" )
            {
                rValue.Value >>= mxStatusIndicator;
            }
        }
    }
}

void* JPEGWriter::GetScanline( long nY )
{
    void* pScanline = nullptr;

    if( mpReadAccess )
    {
        if( mbNative )
        {
            pScanline = mpReadAccess->GetScanline( nY );
        }
        else if( mpBuffer )
        {
            BitmapColor aColor;
            long        nWidth = mpReadAccess->Width();
            sal_uInt8*  pTmp = mpBuffer;

            if( mpReadAccess->HasPalette() )
            {
                Scanline pScanlineRead = mpReadAccess->GetScanline( nY );
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    aColor = mpReadAccess->GetPaletteColor( mpReadAccess->GetIndexFromData( pScanlineRead, nX ) );
                    *pTmp++ = aColor.GetRed();
                    if ( !mbGreys )
                    {
                        *pTmp++ = aColor.GetGreen();
                        *pTmp++ = aColor.GetBlue();
                    }
                }
            }
            else
            {
                Scanline pScanlineRead = mpReadAccess->GetScanline( nY );
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    aColor = mpReadAccess->GetPixelFromData( pScanlineRead, nX );
                    *pTmp++ = aColor.GetRed();
                    if ( !mbGreys )
                    {
                        *pTmp++ = aColor.GetGreen();
                        *pTmp++ = aColor.GetBlue();
                    }
                }
            }

            pScanline = mpBuffer;
        }
    }

    return pScanline;
}

bool JPEGWriter::Write( const Graphic& rGraphic )
{
    bool bRet = false;

    if ( mxStatusIndicator.is() )
    {
        mxStatusIndicator->start( OUString(), 100 );
    }

    // This slightly weird logic is here to match the behaviour in ImpGraphic::ImplGetBitmap
    // and is necessary to match pre-existing behaviour. We should probably pass down the expected
    // background color for alpha from the higher layers.
    Bitmap aGraphicBmp;
    if (rGraphic.GetType() == GraphicType::Bitmap)
        aGraphicBmp = rGraphic.GetBitmapEx().GetBitmap(COL_WHITE);
    else
        aGraphicBmp = rGraphic.GetBitmapEx().GetBitmap();

    if ( mbGreys )
    {
        if ( !aGraphicBmp.Convert( BmpConversion::N8BitGreys ) )
            aGraphicBmp = rGraphic.GetBitmapEx().GetBitmap();
    }

    mpReadAccess = Bitmap::ScopedReadAccess(aGraphicBmp);
    if( mpReadAccess )
    {
        if ( !mbGreys )  // bitmap was not explicitly converted into greyscale,
        {                // check if source is greyscale only
            bool bIsGrey = true;

            long nWidth = mpReadAccess->Width();
            for ( long nY = 0; bIsGrey && ( nY < mpReadAccess->Height() ); nY++ )
            {
                BitmapColor aColor;
                Scanline pScanlineRead = mpReadAccess->GetScanline( nY );
                for( long nX = 0; bIsGrey && ( nX < nWidth ); nX++ )
                {
                    aColor = mpReadAccess->HasPalette() ? mpReadAccess->GetPaletteColor( mpReadAccess->GetIndexFromData( pScanlineRead, nX ) )
                                                : mpReadAccess->GetPixelFromData( pScanlineRead, nX );
                    bIsGrey = ( aColor.GetRed() == aColor.GetGreen() ) && ( aColor.GetRed() == aColor.GetBlue() );
                }
            }
            if ( bIsGrey )
                mbGreys = true;
        }
        if( mpExpWasGrey )
            *mpExpWasGrey = mbGreys;

        if ( mbGreys )
            mbNative = ( mpReadAccess->GetScanlineFormat() == ScanlineFormat::N8BitPal && aGraphicBmp.HasGreyPalette8Bit());
        else
            mbNative = ( mpReadAccess->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb );

        if( !mbNative )
            mpBuffer = new sal_uInt8[ AlignedWidth4Bytes( mbGreys ? mpReadAccess->Width() * 8L : mpReadAccess->Width() * 24L ) ];

        SAL_INFO("vcl", "\nJPEG Export - DPI X: " << rGraphic.GetPPI().getX() << "\nJPEG Export - DPI Y: " << rGraphic.GetPPI().getY());

        bRet = WriteJPEG( this, &mrStream, mpReadAccess->Width(),
                          mpReadAccess->Height(), rGraphic.GetPPI(), mbGreys,
                          mnQuality, maChromaSubsampling, mxStatusIndicator );

        delete[] mpBuffer;
        mpBuffer = nullptr;

        mpReadAccess.reset();
    }
    if ( mxStatusIndicator.is() )
        mxStatusIndicator->end();

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
