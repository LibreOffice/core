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
#include <o3tl/safeint.hxx>

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <jerror.h>

#include <com/sun/star/task/XStatusIndicator.hpp>

extern "C" {
#include "transupp.h"
}

#include "jpeg.h"
#include "JpegReader.hxx"
#include "JpegWriter.hxx"
#include <memory>
#include <unotools/configmgr.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/graphicfilter.hxx>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4324) /* disable to __declspec(align()) aligned warning */
#endif

struct ErrorManagerStruct
{
    jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

extern "C" {

static void errorExit (j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);
    SAL_WARN("vcl.filter", "fatal failure reading JPEG: " << buffer);
    ErrorManagerStruct * error = reinterpret_cast<ErrorManagerStruct *>(cinfo->err);
    longjmp(error->setjmp_buffer, 1);
}

static void outputMessage (j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);
    SAL_WARN("vcl.filter", "failure reading JPEG: " << buffer);
}

}

static int GetWarningLimit()
{
    return utl::ConfigManager::IsFuzzing() ? 5 : 1000;
}

extern "C" {

static void emitMessage (j_common_ptr cinfo, int msg_level)
{
    if (msg_level < 0)
    {
        // ofz#3002 try to retain some degree of recoverability up to some
        // reasonable limit (initially using ImageMagick's current limit of
        // 1000), then bail.
        // https://libjpeg-turbo.org/pmwiki/uploads/About/TwoIssueswiththeJPEGStandard.pdf
        if (++cinfo->err->num_warnings > GetWarningLimit())
            cinfo->err->error_exit(cinfo);
        else
            cinfo->err->output_message(cinfo);
    }
    else if (cinfo->err->trace_level >= msg_level)
        cinfo->err->output_message(cinfo);
}

}

class JpegDecompressOwner
{
public:
    void set(jpeg_decompress_struct *cinfo)
    {
        m_cinfo = cinfo;
    }
    ~JpegDecompressOwner()
    {
        if (m_cinfo != nullptr)
        {
            jpeg_destroy_decompress(m_cinfo);
        }
    }
private:
    jpeg_decompress_struct *m_cinfo = nullptr;
};

class JpegCompressOwner
{
public:
    void set(jpeg_compress_struct *cinfo)
    {
        m_cinfo = cinfo;
    }
    ~JpegCompressOwner()
    {
        if (m_cinfo != nullptr)
        {
            jpeg_destroy_compress(m_cinfo);
        }
    }
private:
    jpeg_compress_struct *m_cinfo = nullptr;
};

struct JpegStuff
{
    jpeg_decompress_struct cinfo;
    ErrorManagerStruct jerr;
    JpegDecompressOwner aOwner;
    std::unique_ptr<BitmapScopedWriteAccess> pScopedAccess;
    std::vector<sal_uInt8> pScanLineBuffer;
    std::vector<sal_uInt8> pCYMKBuffer;
};

static void ReadJPEG(JpegStuff& rContext, JPEGReader* pJPEGReader, void* pInputStream, long* pLines,
              Size const & previewSize, GraphicFilterImportFlags nImportFlags,
              BitmapScopedWriteAccess* ppAccess)
{
    if (setjmp(rContext.jerr.setjmp_buffer))
    {
        return;
    }

    rContext.cinfo.err = jpeg_std_error(&rContext.jerr.pub);
    rContext.jerr.pub.error_exit = errorExit;
    rContext.jerr.pub.output_message = outputMessage;
    rContext.jerr.pub.emit_message = emitMessage;

    jpeg_create_decompress(&rContext.cinfo);
    rContext.aOwner.set(&rContext.cinfo);
    jpeg_svstream_src(&rContext.cinfo, pInputStream);
    SourceManagerStruct *source = reinterpret_cast<SourceManagerStruct*>(rContext.cinfo.src);
    jpeg_read_header(&rContext.cinfo, TRUE);

    rContext.cinfo.scale_num = 1;
    rContext.cinfo.scale_denom = 1;
    rContext.cinfo.output_gamma = 1.0;
    rContext.cinfo.raw_data_out = FALSE;
    rContext.cinfo.quantize_colors = FALSE;

    /* change scale for preview import */
    long nPreviewWidth = previewSize.Width();
    long nPreviewHeight = previewSize.Height();
    if( nPreviewWidth || nPreviewHeight )
    {
        if( nPreviewWidth == 0 )
        {
            nPreviewWidth = (rContext.cinfo.image_width * nPreviewHeight) / rContext.cinfo.image_height;
            if( nPreviewWidth <= 0 )
            {
                nPreviewWidth = 1;
            }
        }
        else if( nPreviewHeight == 0 )
        {
            nPreviewHeight = (rContext.cinfo.image_height * nPreviewWidth) / rContext.cinfo.image_width;
            if( nPreviewHeight <= 0 )
            {
                nPreviewHeight = 1;
            }
        }

        for (rContext.cinfo.scale_denom = 1; rContext.cinfo.scale_denom < 8; rContext.cinfo.scale_denom *= 2)
        {
            if (rContext.cinfo.image_width < nPreviewWidth * rContext.cinfo.scale_denom)
                break;
            if (rContext.cinfo.image_height < nPreviewHeight * rContext.cinfo.scale_denom)
                break;
        }

        if (rContext.cinfo.scale_denom > 1)
        {
            rContext.cinfo.dct_method            = JDCT_FASTEST;
            rContext.cinfo.do_fancy_upsampling   = FALSE;
            rContext.cinfo.do_block_smoothing    = FALSE;
        }
    }

    jpeg_calc_output_dimensions(&rContext.cinfo);

    long nWidth = rContext.cinfo.output_width;
    long nHeight = rContext.cinfo.output_height;

    long nResult = 0;
    if (utl::ConfigManager::IsFuzzing() && (o3tl::checked_multiply(nWidth, nHeight, nResult) || nResult > 4000000))
        return;

    bool bGray = (rContext.cinfo.output_components == 1);

    JPEGCreateBitmapParam aCreateBitmapParam;

    aCreateBitmapParam.nWidth = nWidth;
    aCreateBitmapParam.nHeight = nHeight;

    aCreateBitmapParam.density_unit = rContext.cinfo.density_unit;
    aCreateBitmapParam.X_density = rContext.cinfo.X_density;
    aCreateBitmapParam.Y_density = rContext.cinfo.Y_density;
    aCreateBitmapParam.bGray = bGray;

    const auto bOnlyCreateBitmap = static_cast<bool>(nImportFlags & GraphicFilterImportFlags::OnlyCreateBitmap);
    const auto bUseExistingBitmap = static_cast<bool>(nImportFlags & GraphicFilterImportFlags::UseExistingBitmap);
    bool bBitmapCreated = bUseExistingBitmap;
    if (!bBitmapCreated)
        bBitmapCreated = pJPEGReader->CreateBitmap(aCreateBitmapParam);

    if (bBitmapCreated && !bOnlyCreateBitmap)
    {
        if (nImportFlags & GraphicFilterImportFlags::UseExistingBitmap)
            // ppAccess must be set if this flag is used.
            assert(ppAccess);
        else
            rContext.pScopedAccess.reset(new BitmapScopedWriteAccess(pJPEGReader->GetBitmap()));

        BitmapScopedWriteAccess& pAccess = bUseExistingBitmap ? *ppAccess : *rContext.pScopedAccess;

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
#if defined(JCS_EXTENSIONS)
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
#endif
            if (rContext.cinfo.jpeg_color_space == JCS_YCCK)
                rContext.cinfo.out_color_space = JCS_CMYK;

            if (rContext.cinfo.out_color_space != JCS_CMYK)
                rContext.cinfo.out_color_space = best_out_color_space;

            jpeg_start_decompress(&rContext.cinfo);

            JSAMPLE* aRangeLimit = rContext.cinfo.sample_range_limit;

            rContext.pScanLineBuffer.resize(nWidth * nPixelSize);

            if (rContext.cinfo.out_color_space == JCS_CMYK)
            {
                rContext.pCYMKBuffer.resize(nWidth * 4);
            }

            for (*pLines = 0; *pLines < nHeight && !source->no_data_available; (*pLines)++)
            {
                size_t yIndex = *pLines;

                sal_uInt8* p = (rContext.cinfo.out_color_space == JCS_CMYK) ? rContext.pCYMKBuffer.data() : rContext.pScanLineBuffer.data();
                jpeg_read_scanlines(&rContext.cinfo, reinterpret_cast<JSAMPARRAY>(&p), 1);

                if (rContext.cinfo.out_color_space == JCS_CMYK)
                {
                    // convert CMYK to RGB
                    Scanline pScanline = pAccess->GetScanline(yIndex);
                    for (long cmyk = 0, x = 0; cmyk < nWidth * 4; cmyk += 4, ++x)
                    {
                        int color_C = 255 - rContext.pCYMKBuffer[cmyk + 0];
                        int color_M = 255 - rContext.pCYMKBuffer[cmyk + 1];
                        int color_Y = 255 - rContext.pCYMKBuffer[cmyk + 2];
                        int color_K = 255 - rContext.pCYMKBuffer[cmyk + 3];

                        sal_uInt8 cRed = aRangeLimit[255L - (color_C + color_K)];
                        sal_uInt8 cGreen = aRangeLimit[255L - (color_M + color_K)];
                        sal_uInt8 cBlue = aRangeLimit[255L - (color_Y + color_K)];

                        pAccess->SetPixelOnData(pScanline, x, BitmapColor(cRed, cGreen, cBlue));
                    }
                }
                else
                {
                    pAccess->CopyScanline(yIndex, rContext.pScanLineBuffer.data(), eScanlineFormat, rContext.pScanLineBuffer.size());
                }

                /* PENDING ??? */
                if (rContext.cinfo.err->msg_code == 113)
                    break;
            }

            rContext.pScanLineBuffer.clear();
            rContext.pCYMKBuffer.clear();
        }
        rContext.pScopedAccess.reset();
    }

    if (bBitmapCreated && !bOnlyCreateBitmap)
    {
        jpeg_finish_decompress(&rContext.cinfo);
    }
    else
    {
        jpeg_abort_decompress(&rContext.cinfo);
    }
}

void ReadJPEG( JPEGReader* pJPEGReader, void* pInputStream, long* pLines,
               Size const & previewSize, GraphicFilterImportFlags nImportFlags,
               BitmapScopedWriteAccess* ppAccess )
{
    JpegStuff aContext;
    ReadJPEG(aContext, pJPEGReader, pInputStream, pLines, previewSize, nImportFlags, ppAccess);
}

bool WriteJPEG( JPEGWriter* pJPEGWriter, void* pOutputStream,
                long nWidth, long nHeight, basegfx::B2DSize const & rPPI, bool bGreys,
                long nQualityPercent, long aChromaSubsampling,
                css::uno::Reference<css::task::XStatusIndicator> const & status )
{
    jpeg_compress_struct        cinfo;
    ErrorManagerStruct          jerr;
    void*                       pScanline;
    long                        nY;

    JpegCompressOwner aOwner;

    if ( setjmp( jerr.setjmp_buffer ) )
    {
        return false;
    }

    cinfo.err = jpeg_std_error( &jerr.pub );
    jerr.pub.error_exit = errorExit;
    jerr.pub.output_message = outputMessage;

    jpeg_create_compress( &cinfo );
    aOwner.set(&cinfo);
    jpeg_svstream_dest( &cinfo, pOutputStream );

    cinfo.image_width = static_cast<JDIMENSION>(nWidth);
    cinfo.image_height = static_cast<JDIMENSION>(nHeight);
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
    jpeg_set_quality( &cinfo, static_cast<int>(nQualityPercent), FALSE );

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

    return true;
}

void Transform(void* pInputStream, void* pOutputStream, long nAngle)
{
    jpeg_transform_info aTransformOption;
    JCOPY_OPTION        aCopyOption = JCOPYOPT_ALL;

    jpeg_decompress_struct   aSourceInfo;
    jpeg_compress_struct     aDestinationInfo;
    ErrorManagerStruct       aSourceError;
    ErrorManagerStruct       aDestinationError;

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
    aSourceInfo.err                 = jpeg_std_error(&aSourceError.pub);
    aSourceInfo.err->error_exit     = errorExit;
    aSourceInfo.err->output_message = outputMessage;

    // Compression
    aDestinationInfo.err                 = jpeg_std_error(&aDestinationError.pub);
    aDestinationInfo.err->error_exit     = errorExit;
    aDestinationInfo.err->output_message = outputMessage;

    aDestinationInfo.optimize_coding = TRUE;

    JpegDecompressOwner aDecompressOwner;
    JpegCompressOwner aCompressOwner;

    if (setjmp(aSourceError.setjmp_buffer) || setjmp(aDestinationError.setjmp_buffer))
    {
        jpeg_destroy_decompress(&aSourceInfo);
        jpeg_destroy_compress(&aDestinationInfo);
        return;
    }

    jpeg_create_decompress(&aSourceInfo);
    aDecompressOwner.set(&aSourceInfo);
    jpeg_create_compress(&aDestinationInfo);
    aCompressOwner.set(&aDestinationInfo);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
