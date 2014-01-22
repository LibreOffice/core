/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

#include "PictToBmpFlt.hxx"

bool PICTtoPNG( com::sun::star::uno::Sequence<sal_Int8>& rPictData,
            com::sun::star::uno::Sequence<sal_Int8>& rPngData)
{
#ifdef MAC_OS_X_VERSION_10_6
    return false;
#else // MAC_OS_X_VERSION_10_6
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
        rtl_copyMemory( rPngData.getArray(), ((sal_Int8*)*hPng), nPngSize);
        HUnlock( hPng);
    }

    if( hPict)
        DisposeHandle( hPict);
    if( hPng)
        DisposeHandle( hPng);
    if( pngExporter)
        CloseComponent( pngExporter);

    return (nPngSize > 0);
#endif // MAC_OS_X_VERSION_10_6
}


bool PNGtoPICT( com::sun::star::uno::Sequence<sal_Int8>& rPngData,
               com::sun::star::uno::Sequence<sal_Int8>& rPictData)
{
#ifdef MAC_OS_X_VERSION_10_6
    return false;
#else // MAC_OS_X_VERSION_10_6
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
        rtl_copyMemory( rPictData.getArray(), ((sal_Int8*)*hPict), nPictSize);
        HUnlock( (Handle)hPict);

#if __MAC_OS_X_VERSION_MAX_ALLOWED <= 1060
        // Release the data associated with the picture
        // Note: This function has been deprecated in OSX 10.4 and removed in OSX 10.7
        KillPicture( hPict);
#endif
    }

    if( hPng)
        DisposeHandle( hPng);
    if( pictExporter)
        CloseComponent( pictExporter);

    return (nPictSize > 512);
#endif // MAC_OS_X_VERSION_10_6
}

bool ImageToPNG( com::sun::star::uno::Sequence<sal_Int8>& rImgData,
                 com::sun::star::uno::Sequence<sal_Int8>& rPngData,
                 NSBitmapImageFileType eInFormat)
{
    // short circuit for PNG->PNG request
    if( eInFormat == NSPNGFileType) {
        rPngData = rImgData;
        return true;
    }

    // special handling for old PICT images that are not supported by NSBitmapImage
    if( eInFormat == PICTImageFileType)
        return PICTtoPNG( rImgData, rPngData);

    // let Cocoa's NSBitmapImageRep do the conversion
    NSData* pData = [NSData dataWithBytesNoCopy: (void*)rImgData.getConstArray() length: rImgData.getLength() freeWhenDone: 0];
    if( !pData)
        return false;

    NSBitmapImageRep* pRep =[NSBitmapImageRep imageRepWithData: pData];
        if( !pRep)
        return false;

    NSData* pOut = [pRep representationUsingType: NSPNGFileType properties: nil];
    if( !pOut)
        return false;

    // get the conversion result
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
    // short circuit for PNG->PNG request
    if( eOutFormat == NSPNGFileType) {
        rImgData = rPngData;
        return true;
    }

    // special handling for old PICT images that are not supported by NSBitmapImage
    if( eOutFormat == PICTImageFileType)
        return PNGtoPICT( rPngData, rImgData);

    // let Cocoa's NSBitmapImageRep do the conversion
    NSData* pData = [NSData dataWithBytesNoCopy: const_cast<sal_Int8*>(rPngData.getConstArray()) length: rPngData.getLength() freeWhenDone: 0];
    if( !pData)
        return false;

        NSBitmapImageRep* pRep = [NSBitmapImageRep imageRepWithData: pData];
        if( !pRep)
        return false;

    NSData* pOut = [pRep representationUsingType: eOutFormat properties: nil];
    if( !pOut)
        return false;

    // get the conversion result
    const size_t nImgSize = [pOut length];
    rImgData.realloc( nImgSize);
    [pOut getBytes: rImgData.getArray() length: nImgSize];
    return (nImgSize > 0);
}

