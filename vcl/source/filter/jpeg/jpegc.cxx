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
#include <o3tl/float_int_conversion.hxx>
#include <o3tl/safeint.hxx>

#include <stdio.h>
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
#include <comphelper/configuration.hxx>
#include <vcl/graphicfilter.hxx>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4324) /* disable to __declspec(align()) aligned warning */
#endif

namespace {

struct ErrorManagerStruct
{
    jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;

    ErrorManagerStruct()
    {
        pub = {};
        memset(&setjmp_buffer, 0, sizeof(setjmp_buffer));
    }
};

}

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

extern "C" {

// see also external/libtiff/0001-ofz-54685-Timeout.patch
static void emitMessage (j_common_ptr cinfo, int msg_level)
{
    if (msg_level < 0)
    {
        // https://libjpeg-turbo.org/pmwiki/uploads/About/TwoIssueswiththeJPEGStandard.pdf
        // try to retain some degree of recoverability up to some reasonable
        // limit (initially using ImageMagick's current limit of 1000), then
        // bail.
        constexpr int WarningLimit = 1000;
        static bool bFuzzing = comphelper::IsFuzzing();
        // ofz#50452 due to Timeouts, just abandon fuzzing on any
        // JWRN_NOT_SEQUENTIAL
        if (bFuzzing && cinfo->err->msg_code == JWRN_NOT_SEQUENTIAL)
        {
            cinfo->err->error_exit(cinfo);
            return;
        }
        if (++cinfo->err->num_warnings > WarningLimit)
            cinfo->err->error_exit(cinfo);
        else
            cinfo->err->output_message(cinfo);
    }
    else if (cinfo->err->trace_level >= msg_level)
        cinfo->err->output_message(cinfo);
}

}

namespace {

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
    jpeg_progress_mgr progress;
    ErrorManagerStruct jerr;
    JpegDecompressOwner aOwner;
    std::unique_ptr<BitmapScopedWriteAccess> pScopedAccess;
    std::vector<sal_uInt8> pScanLineBuffer;
    std::vector<sal_uInt8> pCYMKBuffer;
};

// https://github.com/libjpeg-turbo/libjpeg-turbo/issues/284
// https://libjpeg-turbo.org/pmwiki/uploads/About/TwoIssueswiththeJPEGStandard.pdf
#define LIMITSCANS 100

void progress_monitor(j_common_ptr cinfo)
{
    if (cinfo->is_decompressor)
    {
        jpeg_decompress_struct* decompressor = reinterpret_cast<j_decompress_ptr>(cinfo);
        if (decompressor->input_scan_number >= LIMITSCANS)
        {
            SAL_WARN("vcl.filter", "too many progressive scans, cancelling import after: " << decompressor->input_scan_number << " scans");
            errorExit(cinfo);
        }
    }
}

}

static void ReadJPEG(JpegStuff& rContext, JPEGReader* pJPEGReader, void* pInputStream, tools::Long* pLines,
              GraphicFilterImportFlags nImportFlags,
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
    rContext.cinfo.progress = &rContext.progress;
    rContext.progress.progress_monitor = progress_monitor;
    SourceManagerStruct *source = reinterpret_cast<SourceManagerStruct*>(rContext.cinfo.src);
    jpeg_read_header(&rContext.cinfo, TRUE);

    rContext.cinfo.scale_num = 1;
    rContext.cinfo.scale_denom = 1;
    rContext.cinfo.output_gamma = 1.0;
    rContext.cinfo.raw_data_out = FALSE;
    rContext.cinfo.quantize_colors = FALSE;

    jpeg_calc_output_dimensions(&rContext.cinfo);

    tools::Long nWidth = rContext.cinfo.output_width;
    tools::Long nHeight = rContext.cinfo.output_height;

    if (comphelper::IsFuzzing())
    {
        tools::Long nResult = 0;
        if (o3tl::checked_multiply(nWidth, nHeight, nResult) || nResult > 4000000)
            return;
        if (rContext.cinfo.err->num_warnings && (nWidth > 8192 || nHeight > 8192))
            return;
    }

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
            else if (eFinalFormat == ScanlineFormat::N24BitTcBgr)
            {
                best_out_color_space = JCS_EXT_BGR;
                eScanlineFormat = eFinalFormat;
                nPixelSize = 3;
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

            // tdf#138950 allow up to one short read (no_data_available_failures <= 1) to not trigger cancelling import
            for (*pLines = 0; *pLines < nHeight && source->no_data_available_failures <= 1; (*pLines)++)
            {
                size_t yIndex = *pLines;

                sal_uInt8* p = (rContext.cinfo.out_color_space == JCS_CMYK) ? rContext.pCYMKBuffer.data() : rContext.pScanLineBuffer.data();
                jpeg_read_scanlines(&rContext.cinfo, reinterpret_cast<JSAMPARRAY>(&p), 1);

                if (rContext.cinfo.out_color_space == JCS_CMYK)
                {
                    // convert CMYK to RGB
                    Scanline pScanline = pAccess->GetScanline(yIndex);
                    for (tools::Long cmyk = 0, x = 0; cmyk < nWidth * 4; cmyk += 4, ++x)
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

void ReadJPEG( JPEGReader* pJPEGReader, void* pInputStream, tools::Long* pLines,
               GraphicFilterImportFlags nImportFlags,
               BitmapScopedWriteAccess* ppAccess )
{
    JpegStuff aContext;
    ReadJPEG(aContext, pJPEGReader, pInputStream, pLines, nImportFlags, ppAccess);
}

bool WriteJPEG( JPEGWriter* pJPEGWriter, void* pOutputStream,
                tools::Long nWidth, tools::Long nHeight, basegfx::B2DSize const & rPPI, bool bGreys,
                tools::Long nQualityPercent, tools::Long aChromaSubsampling,
                css::uno::Reference<css::task::XStatusIndicator> const & status )
{
    jpeg_compress_struct        cinfo;
    ErrorManagerStruct          jerr;
    void*                       pScanline;
    tools::Long                        nY;

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

    if (o3tl::convertsToAtMost(rPPI.getWidth(), 65535) && o3tl::convertsToAtMost(rPPI.getHeight(), 65535))
    {
        cinfo.density_unit = 1;
        cinfo.X_density = rPPI.getWidth();
        cinfo.Y_density = rPPI.getHeight();
    }
    else
    {
        SAL_WARN("vcl.filter", "ignoring too large PPI (" << rPPI.getWidth() << ", " << rPPI.getHeight() << ")");
    }

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

void Transform(void* pInputStream, void* pOutputStream, Degree10 nAngle)
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
    switch (nAngle.get())
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

    if (setjmp(aSourceError.setjmp_buffer))
    {
        jpeg_destroy_decompress(&aSourceInfo);
        jpeg_destroy_compress(&aDestinationInfo);
        return;
    }
    if (setjmp(aDestinationError.setjmp_buffer))
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
