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

    // CGImageCreateWithMask() only likes masks or greyscale images => convert if needed
    // TODO: isolate in an extra method?
    if( !CGImageIsMask(xMask) || rMask.GetBitCount() != 8)//(CGImageGetColorSpace(xMask) != GetSalData()->mxGraySpace) )
    {
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

    if( !xMask )
        return xImage;

    // combine image and alpha mask
    CGImageRef xMaskedImage = CGImageCreateWithMask( xImage, xMask );
    CFRelease( xMask );
    CFRelease( xImage );
    return xMaskedImage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
