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

#include <memory>

#include <objbase.h>
#include <strmif.h>
#include "interface.hxx"
#include <uuids.h>

// Media Foundation headers
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include "framegrabber.hxx"
#include "player.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/BitmapTools.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <systools/win32/oleauto.hxx>

constexpr OUStringLiteral AVMEDIA_WIN_FRAMEGRABBER_IMPLEMENTATIONNAME = u"com.sun.star.comp.avmedia.FrameGrabber_DirectX";
constexpr OUString AVMEDIA_WIN_FRAMEGRABBER_SERVICENAME = u"com.sun.star.media.FrameGrabber_DirectX"_ustr;
constexpr LONGLONG SEEK_TOLERANCE = 10000000;
constexpr LONGLONG MAX_FRAMES_TO_SKIP = 10;

using namespace ::com::sun::star;

namespace avmedia::win {


FrameGrabber::FrameGrabber( const OUString& rURL, UINT32 nFrameWidth, UINT32 nFrameHeight )
    : sal::systools::CoInitializeGuard(COINIT_APARTMENTTHREADED, false,
                                       sal::systools::CoInitializeGuard::WhenFailed::NoThrow)
{
    maURL = rURL;
    mnFrameWidth = nFrameWidth;
    mnFrameHeight = nFrameHeight;
}

FrameGrabber::~FrameGrabber() = default;

uno::Reference< graphic::XGraphic > SAL_CALL FrameGrabber::grabFrame( double fMediaTime )
{
    uno::Reference< graphic::XGraphic > xRet;

    if (mnFrameWidth && mnFrameHeight &&
        SUCCEEDED(MFStartup(MF_VERSION)))
    {
        HRESULT hr = S_OK;
        IMFAttributes* pAttributes = nullptr;

        // Configure the source reader to perform video processing.
        //
        // This includes:
        //   - YUV to RGB-32
        //   - Software deinterlace
        if (SUCCEEDED(MFCreateAttributes(&pAttributes, 1)))
        {
            hr = pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
        }

        // Create the source reader from the URL.
        IMFSourceReader* pReader = nullptr; // Create the source reader.
        if (SUCCEEDED(hr) && SUCCEEDED(MFCreateSourceReaderFromURL(o3tl::toW(maURL.getStr()), pAttributes, &pReader)))
        {
            IMFMediaType* pType = nullptr;

            // Configure the source reader to give us progressive RGB32 frames.
            // The source reader will load the decoder if needed.
            if (SUCCEEDED(MFCreateMediaType(&pType)))
                hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);

            if (SUCCEEDED(hr))
                hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

            if (SUCCEEDED(hr))
            {
                hr = pReader->SetCurrentMediaType(
                    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                    nullptr,
                    pType
                );
            }

            // Ensure the stream is selected.
            if (SUCCEEDED(hr))
            {
                hr = pReader->SetStreamSelection(
                    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                    TRUE
                );
            }

            SafeRelease(&pType);

            if (SUCCEEDED(hr) && fMediaTime > 0.0)
            {
                PROPVARIANT var;
                PropVariantInit(&var);

                var.vt = VT_I8;
                var.hVal.QuadPart = fMediaTime;

                hr = pReader->SetCurrentPosition(GUID_NULL, var);
            }

            IMFSample* pSampleTmp = nullptr;

            if (SUCCEEDED(hr))
            {
                DWORD dwFlags = 0;
                DWORD cSkipped = 0; // Number of skipped frames
                while (true)
                {
                    hr = pReader->ReadSample(
                        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                        0,
                        nullptr,
                        &dwFlags,
                        nullptr,
                        &pSampleTmp
                    );

                    if (FAILED(hr) || (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM))
                        break;

                    if (pSampleTmp == nullptr)
                        continue;

                    LONGLONG hnsTimeStamp = 0;
                    if (SUCCEEDED(pSampleTmp->GetSampleTime(&hnsTimeStamp)))
                    {
                        // Keep going until we get a frame that is within tolerance of the
                        // desired seek position, or until we skip MAX_FRAMES_TO_SKIP frames.

                        // During this process, we might reach the end of the file, so we
                        // always cache the last sample that we got (pSample).

                        if ((cSkipped < MAX_FRAMES_TO_SKIP) &&
                            (hnsTimeStamp + SEEK_TOLERANCE < fMediaTime))
                        {
                            SafeRelease(&pSampleTmp);

                            ++cSkipped;
                            continue;
                        }
                    }

                    fMediaTime = hnsTimeStamp;
                    break;
                }
            }

            // We got a sample. Hold onto it.
            IMFMediaBuffer* pBuffer = nullptr;
            BYTE* pBitmapData = nullptr;  // Bitmap data
            DWORD cbBitmapData = 0;       // Size of data, in bytes
            if (pSampleTmp && SUCCEEDED(pSampleTmp->ConvertToContiguousBuffer(&pBuffer)))
            {
                if (SUCCEEDED(pBuffer->Lock(&pBitmapData, nullptr, &cbBitmapData)) && cbBitmapData)
                {
                    BitmapEx aBitmapEx = vcl::bitmap::CreateFromData(pBitmapData, mnFrameWidth, mnFrameHeight, mnFrameWidth * 4, /*nBitsPerPixel*/32, true);
                    Graphic aGraphic(aBitmapEx);
                    xRet = aGraphic.GetXGraphic();
                }
                pBuffer->Unlock();
            }

            SafeRelease(&pBuffer);
            SafeRelease(&pSampleTmp);
        }

        SafeRelease(&pAttributes);
        SafeRelease(&pReader);
        // Shut down Media Foundation.
        MFShutdown();
    }

    return xRet;
}


OUString SAL_CALL FrameGrabber::getImplementationName(  )
{
    return AVMEDIA_WIN_FRAMEGRABBER_IMPLEMENTATIONNAME;
}


sal_Bool SAL_CALL FrameGrabber::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL FrameGrabber::getSupportedServiceNames(  )
{
    return { AVMEDIA_WIN_FRAMEGRABBER_SERVICENAME };
}

} // namespace avmedia::win


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
