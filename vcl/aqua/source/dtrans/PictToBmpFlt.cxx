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

/* This is a work-around to prevent 'deprecated' warning for 'KillPicture' API
   Hopefully we can get rid of this whole code again when the OOo PICT filter
   are good enough to be used see #i78953 thus this hack would vanish to again.
 */
#include <premac.h>
#include <AvailabilityMacros.h>
#undef DEPRECATED_ATTRIBUTE
#define DEPRECATED_ATTRIBUTE

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <postmac.h>
#include <string.h>

#include "PictToBmpFlt.hxx"

bool PICTtoPNG( com::sun::star::uno::Sequence<sal_Int8>& rPictData,
            com::sun::star::uno::Sequence<sal_Int8>& rPngData)
{
#ifdef __LP64__
    // FIXME
    (void) rPictData;
    (void) rPngData;
    return false;
#else
    ComponentInstance pngExporter = NULL;
    if( OpenADefaultComponent( GraphicsExporterComponentType, kQTFileTypePNG, &pngExporter) != noErr)
        return false;

    Handle hPict = NULL;
    if( PtrToHand( rPictData.getArray(), &hPict, rPictData.getLength()) != noErr)
        hPict = NULL;

    Handle hPng = NULL;
    if( hPict && GraphicsExportSetInputPicture( pngExporter, (PicHandle)hPict) == noErr)
        hPng = NewHandleClear(0);

    size_t nPngSize = 0;
    if( hPng
    && (GraphicsExportSetOutputHandle( pngExporter, hPng) == noErr)
    && (GraphicsExportDoExport( pngExporter, NULL) == noErr))
    {
        nPngSize = GetHandleSize( hPng);
        rPngData.realloc( nPngSize);

        HLock( hPng);
        memmove( rPngData.getArray(), ((sal_Int8*)*hPng), nPngSize);
        HUnlock( hPng);
    }

    if( hPict)
        DisposeHandle( hPict);
    if( hPng)
        DisposeHandle( hPng);
    if( pngExporter)
        CloseComponent( pngExporter);

    return (nPngSize > 0);
#endif
}

#if MACOSX_SDK_VERSION >= 1070

// This whole thing needs to be rewritten I guess. Or does this code
// even get invoked on current OSes? Given that KillPicture() was
// deprecated already in 10.4, back when somebody was actually working
// on this code, hopefully knowing what he/she was doing, did he/she
// really not pay attention and notice that this stuff is going to go
// away?

extern "C" {
extern void KillPicture(PicHandle myPicture);
}

#endif

bool PNGtoPICT( com::sun::star::uno::Sequence<sal_Int8>& rPngData,
               com::sun::star::uno::Sequence<sal_Int8>& rPictData)
{
#ifdef __LP64__
    // FIXME
    (void) rPngData;
    (void) rPictData;
    return false;
#else
    ComponentInstance pictExporter;
    if( OpenADefaultComponent( GraphicsImporterComponentType, kQTFileTypePNG, &pictExporter) != noErr)
        return false;

    Handle hPng = NULL;
    if( PtrToHand( rPngData.getArray(), &hPng, rPngData.getLength()) != noErr)
        hPng = NULL;

    size_t nPictSize = 0;
    PicHandle hPict = NULL;
    if( hPng
    && (GraphicsImportSetDataHandle( pictExporter, hPng) == noErr)
    && (GraphicsImportGetAsPicture( pictExporter, &hPict) == noErr))
    {
        nPictSize = GetHandleSize( (Handle)hPict);
        rPictData.realloc( nPictSize);

        HLock( (Handle)hPict);
        memmove( rPictData.getArray(), ((sal_Int8*)*hPict), nPictSize);
        HUnlock( (Handle)hPict);

        // Release the data associated with the picture
        // Note: This function is deprecated in Mac OSX 10.4
        KillPicture( hPict);
    }

    if( hPng)
        DisposeHandle( hPng);
    if( pictExporter)
        CloseComponent( pictExporter);

    return (nPictSize > 512);
#endif
}

bool ImageToPNG( com::sun::star::uno::Sequence<sal_Int8>& rImgData,
                 com::sun::star::uno::Sequence<sal_Int8>& rPngData,
                 NSBitmapImageFileType eInFormat)
{
    if( eInFormat == PICTImageFileType)
        return PICTtoPNG( rImgData, rPngData);

    NSData* pData = [NSData dataWithBytesNoCopy: (void*)rImgData.getConstArray() length: rImgData.getLength() freeWhenDone: 0];
    if( !pData)
        return false;

    NSBitmapImageRep* pRep =[NSBitmapImageRep imageRepWithData: pData];
        if( !pRep)
        return false;

    NSData* pOut = [pRep representationUsingType: NSPNGFileType properties: nil];
    if( !pOut)
        return false;

    const size_t nPngSize = [pOut length];
    rPngData.realloc( nPngSize);
    [pOut getBytes: rPngData.getArray() length: nPngSize];
    return (nPngSize > 0);
}

bool PNGToImage( com::sun::star::uno::Sequence<sal_Int8>& rPngData,
                 com::sun::star::uno::Sequence<sal_Int8>& rImgData,
                 NSBitmapImageFileType eOutFormat
                )
{
    if( eOutFormat == PICTImageFileType)
        return PNGtoPICT( rPngData, rImgData);

    NSData* pData = [NSData dataWithBytesNoCopy: const_cast<sal_Int8*>(rPngData.getConstArray()) length: rPngData.getLength() freeWhenDone: 0];
    if( !pData)
        return false;

        NSBitmapImageRep* pRep = [NSBitmapImageRep imageRepWithData: pData];
        if( !pRep)
        return false;

    NSData* pOut = [pRep representationUsingType: eOutFormat properties: nil];
    if( !pOut)
        return false;

    const size_t nImgSize = [pOut length];
    rImgData.realloc( nImgSize);
    [pOut getBytes: rImgData.getArray() length: nImgSize];
    return (nImgSize > 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
