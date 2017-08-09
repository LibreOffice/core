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

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <jerror.h>

#include <com/sun/star/task/XStatusIndicator.hpp>

extern "C" {
#include "transupp.h"
}

#include "jpeg.h"
#include <JpegReader.hxx>
#include <JpegWriter.hxx>
#include <memory>
#include <vcl/bitmapaccess.hxx>
#include <vcl/graphicfilter.hxx>

extern "C" void errorExit (j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);
    SAL_WARN("vcl.filter", "fatal failure reading JPEG: " << buffer);
    throw css::uno::RuntimeException(OUString(buffer, strlen(buffer), RTL_TEXTENCODING_ASCII_US));
}

extern "C" void outputMessage (j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);
    SAL_WARN("vcl.filter", "failure reading JPEG: " << buffer);
}

class JpegDecompressOwner
{
public:
    JpegDecompressOwner(jpeg_decompress_struct &cinfo) : m_cinfo(cinfo)
    {
    }
    ~JpegDecompressOwner()
    {
        jpeg_destroy_decompress(&m_cinfo);
    }
private:
    jpeg_decompress_struct &m_cinfo;
};

class JpegCompressOwner
{
public:
    JpegCompressOwner(jpeg_compress_struct &cinfo) : m_cinfo(cinfo)
    {
    }
    ~JpegCompressOwner()
    {
        jpeg_destroy_compress(&m_cinfo);
    }
private:
    jpeg_compress_struct &m_cinfo;
};

void ReadJPEG( JPEGReader* pJPEGReader, void* pInputStream, long* pLines,
               Size const & previewSize, GraphicFilterImportFlags nImportFlags,
               Bitmap::ScopedWriteAccess* ppAccess )
{
    try
    {
        jpeg_decompress_struct cinfo;
        jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error( &jerr );
        jerr.error_exit = errorExit;
        jerr.output_message = outputMessage;

        jpeg_create_decompress( &cinfo );
        JpegDecompressOwner aOwner(cinfo);
        jpeg_svstream_src( &cinfo, pInputStream );
        SourceManagerStruct *source = reinterpret_cast<SourceManagerStruct*>(cinfo.src);
        jpeg_read_header( &cinfo, TRUE );

        cinfo.scale_num = 1;
        cinfo.scale_denom = 1;
        cinfo.output_gamma = 1.0;
        cinfo.raw_data_out = FALSE;
        cinfo.quantize_colors = FALSE;

        /* change scale for preview import */
        long nPreviewWidth = previewSize.Width();
        long nPreviewHeight = previewSize.Height();
        if( nPreviewWidth || nPreviewHeight )
        {
            if( nPreviewWidth == 0 )
            {
                nPreviewWidth = ( cinfo.image_width * nPreviewHeight ) / cinfo.image_height;
                if( nPreviewWidth <= 0 )
                {
                    nPreviewWidth = 1;
                }
            }
            else if( nPreviewHeight == 0 )
            {
                nPreviewHeight = ( cinfo.image_height * nPreviewWidth ) / cinfo.image_width;
                if( nPreviewHeight <= 0 )
                {
                    nPreviewHeight = 1;
                }
            }

            for( cinfo.scale_denom = 1; cinfo.scale_denom < 8; cinfo.scale_denom *= 2 )
            {
                if( cinfo.image_width < nPreviewWidth * cinfo.scale_denom )
                    break;
                if( cinfo.image_height < nPreviewHeight * cinfo.scale_denom )
                    break;
            }

            if( cinfo.scale_denom > 1 )
            {
                cinfo.dct_method            = JDCT_FASTEST;
                cinfo.do_fancy_upsampling   = FALSE;
                cinfo.do_block_smoothing    = FALSE;
            }
        }

        jpeg_calc_output_dimensions(&cinfo);

        long nWidth = cinfo.output_width;
        long nHeight = cinfo.output_height;

        bool bGray = (cinfo.output_components == 1);

        JPEGCreateBitmapParam aCreateBitmapParam;

        aCreateBitmapParam.nWidth = nWidth;
        aCreateBitmapParam.nHeight = nHeight;

        aCreateBitmapParam.density_unit = cinfo.density_unit;
        aCreateBitmapParam.X_density = cinfo.X_density;
        aCreateBitmapParam.Y_density = cinfo.Y_density;
        aCreateBitmapParam.bGray = bGray;

        const auto bOnlyCreateBitmap = static_cast<bool>(nImportFlags & GraphicFilterImportFlags::OnlyCreateBitmap);
        const auto bUseExistingBitmap = static_cast<bool>(nImportFlags & GraphicFilterImportFlags::UseExistingBitmap);
        bool bBitmapCreated = bUseExistingBitmap;
        if (!bBitmapCreated)
            bBitmapCreated = pJPEGReader->CreateBitmap(aCreateBitmapParam);

        if (bBitmapCreated && !bOnlyCreateBitmap)
        {
            std::unique_ptr<Bitmap::ScopedWriteAccess> pScopedAccess;

            if (nImportFlags & GraphicFilterImportFlags::UseExistingBitmap)
                // ppAccess must be set if this flag is used.
                assert(ppAccess);
            else
                pScopedAccess.reset(new Bitmap::ScopedWriteAccess(pJPEGReader->GetBitmap()));

            Bitmap::ScopedWriteAccess& pAccess = bUseExistingBitmap ? *ppAccess : *pScopedAccess.get();

            if (pAccess)
            {
                int nPixelSize = 3;
                J_COLOR_SPACE best_out_color_space = JCS_RGB;
                ScanlineFormat eScanlineFormat = ScanlineFormat::N24BitTcRgb;
                ScanlineFormat eFinalFormat = pAccess->GetScanlineFormat();

                if (bGray)
                {
                    best_out_color_space = JCS_GRAYSCALE;
                    eScanlineFormat = ScanlineFormat::N8BitPal;
                    nPixelSize = 1;
                }
                else if (eFinalFormat == ScanlineFormat::N32BitTcBgra)
                {
                    best_out_color_space = JCS_EXT_BGRA;
                    eScanlineFormat = eFinalFormat;
                    nPixelSize = 4;
                }
                else if (eFinalFormat == ScanlineFormat::N32BitTcRgba)
                {
                    best_out_color_space = JCS_EXT_RGBA;
                    eScanlineFormat = eFinalFormat;
                    nPixelSize = 4;
                }
                else if (eFinalFormat == ScanlineFormat::N32BitTcArgb)
                {
                    best_out_color_space = JCS_EXT_ARGB;
                    eScanlineFormat = eFinalFormat;
                    nPixelSize = 4;
                }

                if (cinfo.jpeg_color_space == JCS_YCCK)
                    cinfo.out_color_space = JCS_CMYK;

                if (cinfo.out_color_space != JCS_CMYK)
                    cinfo.out_color_space = best_out_color_space;

                jpeg_start_decompress(&cinfo);

                JSAMPLE* aRangeLimit = cinfo.sample_range_limit;

                std::vector<sal_uInt8> pScanLineBuffer(nWidth * nPixelSize);
                std::vector<sal_uInt8> pCYMKBuffer;

                if (cinfo.out_color_space == JCS_CMYK)
                {
                    pCYMKBuffer.resize(nWidth * 4);
                }

                for (*pLines = 0; *pLines < nHeight && !source->no_data_available; (*pLines)++)
                {
                    size_t yIndex = *pLines;

                    sal_uInt8* p = (cinfo.out_color_space == JCS_CMYK) ? pCYMKBuffer.data() : pScanLineBuffer.data();
                    jpeg_read_scanlines(&cinfo, reinterpret_cast<JSAMPARRAY>(&p), 1);

                    if (cinfo.out_color_space == JCS_CMYK)
                    {
                        // convert CMYK to RGB
                        for (long cmyk = 0, x = 0; cmyk < nWidth * 4; cmyk += 4, ++x)
                        {
                            int color_C = 255 - pCYMKBuffer[cmyk + 0];
                            int color_M = 255 - pCYMKBuffer[cmyk + 1];
                            int color_Y = 255 - pCYMKBuffer[cmyk + 2];
                            int color_K = 255 - pCYMKBuffer[cmyk + 3];

                            sal_uInt8 cRed = aRangeLimit[255L - (color_C + color_K)];
                            sal_uInt8 cGreen = aRangeLimit[255L - (color_M + color_K)];
                            sal_uInt8 cBlue = aRangeLimit[255L - (color_Y + color_K)];

                            pAccess->SetPixel(yIndex, x, BitmapColor(cRed, cGreen, cBlue));
                        }
                    }
                    else
                    {
                        pAccess->CopyScanline(yIndex, pScanLineBuffer.data(), eScanlineFormat, pScanLineBuffer.size());
                    }

                    /* PENDING ??? */
                    if (cinfo.err->msg_code == 113)
                        break;
                }
            }
        }

        if (bBitmapCreated && !bOnlyCreateBitmap)
        {
            jpeg_finish_decompress( &cinfo );
        }
        else
        {
            jpeg_abort_decompress( &cinfo );
        }
    }
    catch (const css::uno::RuntimeException&)
    {
    }
}

bool WriteJPEG( JPEGWriter* pJPEGWriter, void* pOutputStream,
                long nWidth, long nHeight, basegfx::B2DSize const & rPPI, bool bGreys,
                long nQualityPercent, long aChromaSubsampling,
                css::uno::Reference<css::task::XStatusIndicator> const & status )
{
    try
    {
        jpeg_compress_struct        cinfo;
        jpeg_error_mgr              jerr;
        void*                       pScanline;
        long                        nY;

        cinfo.err = jpeg_std_error( &jerr );
        jerr.error_exit = errorExit;
        jerr.output_message = outputMessage;

        jpeg_create_compress( &cinfo );
        JpegCompressOwner aOwner(cinfo);
        jpeg_svstream_dest( &cinfo, pOutputStream );

        cinfo.image_width = (JDIMENSION) nWidth;
        cinfo.image_height = (JDIMENSION) nHeight;
        if ( bGreys )
        {
            cinfo.input_components = 1;
            cinfo.in_color_space = JCS_GRAYSCALE;
        }
        else
        {
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;
        }

        jpeg_set_defaults( &cinfo );
        jpeg_set_quality( &cinfo, (int) nQualityPercent, FALSE );

        cinfo.density_unit = 1;
        cinfo.X_density = rPPI.getX();
        cinfo.Y_density = rPPI.getY();

        if ( ( nWidth > 128 ) || ( nHeight > 128 ) )
            jpeg_simple_progression( &cinfo );

        if (aChromaSubsampling == 1) // YUV 4:4:4
        {
            cinfo.comp_info[0].h_samp_factor = 1;
            cinfo.comp_info[0].v_samp_factor = 1;
        }
        else if (aChromaSubsampling == 2) // YUV 4:2:2
        {
            cinfo.comp_info[0].h_samp_factor = 2;
            cinfo.comp_info[0].v_samp_factor = 1;
        }
        else if (aChromaSubsampling == 3) // YUV 4:2:0
        {
            cinfo.comp_info[0].h_samp_factor = 2;
            cinfo.comp_info[0].v_samp_factor = 2;
        }

        jpeg_start_compress( &cinfo, TRUE );

        for( nY = 0; nY < nHeight; nY++ )
        {
            pScanline = pJPEGWriter->GetScanline( nY );

            if( pScanline )
            {
                jpeg_write_scanlines( &cinfo, reinterpret_cast<JSAMPARRAY>(&pScanline), 1 );
            }

            if( status.is() )
            {
                status->setValue( nY * 100L / nHeight );
            }
        }

        jpeg_finish_compress(&cinfo);
    }
    catch (const css::uno::RuntimeException&)
    {
        return false;
    }
    return true;
}

void Transform(void* pInputStream, void* pOutputStream, long nAngle)
{
    try
    {
        jpeg_transform_info aTransformOption;
        JCOPY_OPTION        aCopyOption = JCOPYOPT_ALL;

        jpeg_decompress_struct   aSourceInfo;
        jpeg_compress_struct     aDestinationInfo;
        jpeg_error_mgr           aSourceError;
        jpeg_error_mgr           aDestinationError;

        jvirt_barray_ptr* aSourceCoefArrays      = nullptr;
        jvirt_barray_ptr* aDestinationCoefArrays = nullptr;

        aTransformOption.force_grayscale = FALSE;
        aTransformOption.trim            = FALSE;
        aTransformOption.perfect         = FALSE;
        aTransformOption.crop            = FALSE;

        // Angle to transform option
        // 90 Clockwise = 270 Counterclockwise
        switch (nAngle)
        {
            case 2700:
                aTransformOption.transform  = JXFORM_ROT_90;
                break;
            case 1800:
                aTransformOption.transform  = JXFORM_ROT_180;
                break;
            case 900:
                aTransformOption.transform  = JXFORM_ROT_270;
                break;
            default:
                aTransformOption.transform  = JXFORM_NONE;
        }

        // Decompression
        aSourceInfo.err                 = jpeg_std_error(&aSourceError);
        aSourceInfo.err->error_exit     = errorExit;
        aSourceInfo.err->output_message = outputMessage;

        // Compression
        aDestinationInfo.err                 = jpeg_std_error(&aDestinationError);
        aDestinationInfo.err->error_exit     = errorExit;
        aDestinationInfo.err->output_message = outputMessage;

        aDestinationInfo.optimize_coding = TRUE;

        jpeg_create_decompress(&aSourceInfo);
        JpegDecompressOwner aDecompressOwner(aSourceInfo);
        jpeg_create_compress(&aDestinationInfo);
        JpegCompressOwner aCompressOwner(aDestinationInfo);

        jpeg_svstream_src (&aSourceInfo, pInputStream);

        jcopy_markers_setup(&aSourceInfo, aCopyOption);
        jpeg_read_header(&aSourceInfo, TRUE);
        jtransform_request_workspace(&aSourceInfo, &aTransformOption);

        aSourceCoefArrays = jpeg_read_coefficients(&aSourceInfo);
        jpeg_copy_critical_parameters(&aSourceInfo, &aDestinationInfo);

        aDestinationCoefArrays = jtransform_adjust_parameters(&aSourceInfo, &aDestinationInfo, aSourceCoefArrays, &aTransformOption);
        jpeg_svstream_dest (&aDestinationInfo, pOutputStream);

        // Compute optimal Huffman coding tables instead of precomputed tables
        aDestinationInfo.optimize_coding = TRUE;
        jpeg_write_coefficients(&aDestinationInfo, aDestinationCoefArrays);
        jcopy_markers_execute(&aSourceInfo, &aDestinationInfo, aCopyOption);
        jtransform_execute_transformation(&aSourceInfo, &aDestinationInfo, aSourceCoefArrays, &aTransformOption);

        jpeg_finish_compress(&aDestinationInfo);

        jpeg_finish_decompress(&aSourceInfo);
    }
    catch (const css::uno::RuntimeException&)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
