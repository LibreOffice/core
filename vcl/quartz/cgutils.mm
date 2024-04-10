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

#include <quartz/cgutils.h>

#include <salbmp.hxx>
#ifdef MACOSX
#include <osx/saldata.hxx>
#else
#include <ios/iosinst.hxx>
#endif

#ifdef MACOSX
#include <premac.h>
#include <Metal/Metal.h>
#include <postmac.h>
#endif

static void CFRTLFree(void* /*info*/, const void* data, size_t /*size*/)
{
    std::free( const_cast<void*>(data) );
}

CGImageRef CreateWithSalBitmapAndMask( const SalBitmap& rBitmap, const SalBitmap& rMask, int nX, int nY, int nWidth, int nHeight )
{
    CGImageRef xImage( rBitmap.CreateCroppedImage( nX, nY, nWidth, nHeight ) );
    if( !xImage )
        return nullptr;

    CGImageRef xMask = rMask.CreateCroppedImage( nX, nY, nWidth, nHeight );
    if( !xMask )
        return xImage;

    // If xMask is an image (i.e. not a mask), it must be greyscale - a requirement of the
    // CGImageCreateWithMask() function.
    if( !CGImageIsMask(xMask) && CGImageGetColorSpace(xMask) != GetSalData()->mxGraySpace )
    {
        CGImageRef xGrayMask = CGImageCreateCopyWithColorSpace(xMask, GetSalData()->mxGraySpace);
        if (xGrayMask)
        {
            CFRelease(xMask);
            xMask = xGrayMask;
        }
        else
        {
            // Many gallery images will fail to be converted to a grayscale
            // colorspace so fall back to old mask creation code
            const CGRect xImageRect=CGRectMake( 0, 0, nWidth, nHeight );//the rect has no offset

            // create the alpha mask image fitting our image
            // TODO: is caching the full mask or the subimage mask worth it?
            int nMaskBytesPerRow = ((nWidth + 3) & ~3);
            void* pMaskMem = std::malloc( nMaskBytesPerRow * nHeight );
            CGContextRef xMaskContext = CGBitmapContextCreate( pMaskMem,
                nWidth, nHeight, 8, nMaskBytesPerRow, GetSalData()->mxGraySpace, kCGImageAlphaNone );
            CGContextDrawImage( xMaskContext, xImageRect, xMask );
            CFRelease( xMask );
            CGDataProviderRef xDataProvider( CGDataProviderCreateWithData( nullptr,
            pMaskMem, nHeight * nMaskBytesPerRow, &CFRTLFree ) );

            static const CGFloat* pDecode = nullptr;
            xMask = CGImageMaskCreate( nWidth, nHeight, 8, 8, nMaskBytesPerRow, xDataProvider, pDecode, false );
            CFRelease( xDataProvider );
            CFRelease( xMaskContext );
        }
    }

    if( !xMask )
        return xImage;

    // combine image and alpha mask
    CGImageRef xMaskedImage = CGImageCreateWithMask( xImage, xMask );
    CFRelease( xMask );
    CFRelease( xImage );
    return xMaskedImage;
}

#ifdef MACOSX

bool DefaultMTLDeviceIsSupported()
{
    id<MTLDevice> pMetalDevice = MTLCreateSystemDefaultDevice();
    if (!pMetalDevice || !pMetalDevice.name)
    {
        SAL_WARN("vcl.skia", "MTLCreateSystemDefaultDevice() returned nil");
        return false;
    }

    SAL_WARN("vcl.skia", "Default MTLDevice is \"" << [pMetalDevice.name UTF8String] << "\"");

    bool bRet = true;

    // tdf#156881 Disable Metal with AMD Radeon Pro 5XXX GPUs on macOS Catalina
    // When running macOS Catalina on a 2019 MacBook Pro, unexpected drawing
    // artifacts are drawn so disable Metal for the AMD Radeon Pro GPUs listed
    // for that model in https://support.apple.com/kb/SP809.
    if (@available(macOS 11, *))
    {
        // No known problems with macOS Big Sur or later
    }
    else
    {
       static NSString* pAMDRadeonPro5300Prefix = @"AMD Radeon Pro 5300M";
       static NSString* pAMDRadeonPro5500Prefix = @"AMD Radeon Pro 5500M";
       if ([pMetalDevice.name hasPrefix:pAMDRadeonPro5300Prefix] || [pMetalDevice.name hasPrefix:pAMDRadeonPro5500Prefix])
           bRet = false;
    }

    if (bRet)
    {
        // tdf#160590 Disable Metal with Intel HD Graphics 6000
        // Releasing a Metal buffer resource hangs when fetching pixels from a
        // Skia surface on this Intel MacBook Air built-in GPU.
       static NSString* pIntelHDGraphics6000Prefix = @"Intel(R) Iris(TM) Graphics 6000";
       if ([pMetalDevice.name hasPrefix:pIntelHDGraphics6000Prefix])
           bRet = false;
    }

    [pMetalDevice release];
    return bRet;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
