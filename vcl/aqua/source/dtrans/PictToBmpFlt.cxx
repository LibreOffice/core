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

bool PICTtoBMP(com::sun::star::uno::Sequence<sal_Int8>& aPict,
               com::sun::star::uno::Sequence<sal_Int8>& aBmp)
{

  bool result = false;

#ifdef __LP64__
  // FIXME
  (void) aPict;
  (void) aBmp;
#else
  ComponentInstance bmpExporter;
  if (OpenADefaultComponent(GraphicsExporterComponentType,
                            kQTFileTypeBMP,
                            &bmpExporter) != noErr)
    {
      return result;
    }

  Handle hPict;
  if (PtrToHand(aPict.getArray(), &hPict, aPict.getLength()) != noErr)
    {
      return result;
    }

  Handle hBmp;
  if ((GraphicsExportSetInputPicture(bmpExporter, (PicHandle)hPict) != noErr) ||
      ((hBmp = NewHandleClear(0)) == NULL))
    {
      CloseComponent(bmpExporter);
      DisposeHandle(hPict);
      return result;
    }

  if ((GraphicsExportSetOutputHandle(bmpExporter, hBmp) == noErr) &&
      (GraphicsExportDoExport(bmpExporter, NULL) == noErr))
    {
      size_t sz = GetHandleSize(hBmp);
      aBmp.realloc(sz);

      HLock(hBmp);
      memcpy(aBmp.getArray(), ((sal_Int8*)*hBmp), sz);
      HUnlock(hBmp);

      result = true;
    }

  DisposeHandle(hPict);
  DisposeHandle(hBmp);
  CloseComponent(bmpExporter);
#endif
  return result;
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

bool BMPtoPICT(com::sun::star::uno::Sequence<sal_Int8>& aBmp,
               com::sun::star::uno::Sequence<sal_Int8>& aPict)
{
  bool result = false;

  Handle hBmp;
  if ((PtrToHand(aBmp.getArray(), &hBmp, aBmp.getLength()) != noErr))
    {
      return result;
    }

#ifdef __LP64__
  // FIXME
  (void) aPict;
#else
  ComponentInstance pictExporter;

  if (OpenADefaultComponent(GraphicsImporterComponentType,
                            kQTFileTypeBMP,
                            &pictExporter) != noErr)
    {
      DisposeHandle(hBmp);
      return result;
    }

  if (GraphicsImportSetDataHandle(pictExporter, hBmp) != noErr)
    {
      DisposeHandle(hBmp);
      CloseComponent(pictExporter);
      return result;
    }

  PicHandle hPict;
  if (GraphicsImportGetAsPicture(pictExporter, &hPict) == noErr)
    {
      size_t sz = GetHandleSize((Handle)hPict);
      aPict.realloc(sz);

      HLock((Handle)hPict);
      memcpy(aPict.getArray(), ((sal_Int8*)*hPict), sz);
      HUnlock((Handle)hPict);

      // Release the data associated with the picture
      // Note: This function is deprecated in Mac OS X
      // 10.4.

      KillPicture(hPict);

      result = true;
    }

  DisposeHandle(hBmp);
  CloseComponent(pictExporter);
#endif
  return result;
}

bool ImageToBMP( com::sun::star::uno::Sequence<sal_Int8>& aPict,
                 com::sun::star::uno::Sequence<sal_Int8>& aBmp,
                 NSBitmapImageFileType eInFormat)
{
    if( eInFormat == PICTImageFileType )
        return PICTtoBMP( aPict, aBmp );

    bool bResult = false;

    NSData* pData = [NSData dataWithBytesNoCopy: (void*)aPict.getConstArray() length: aPict.getLength() freeWhenDone: 0];
    if( pData )
    {
        NSBitmapImageRep* pRep = [NSBitmapImageRep imageRepWithData: pData];
        if( pRep )
        {
            NSData* pOut = [pRep representationUsingType: NSBMPFileType properties: nil];
            if( pOut )
            {
                aBmp.realloc( [pOut length] );
                [pOut getBytes: aBmp.getArray() length: aBmp.getLength()];
                bResult = (aBmp.getLength() != 0);
            }
        }
    }

    return bResult;
}

bool BMPToImage( com::sun::star::uno::Sequence<sal_Int8>& aBmp,
                 com::sun::star::uno::Sequence<sal_Int8>& aPict,
                 NSBitmapImageFileType eOutFormat
                )
{
    if( eOutFormat == PICTImageFileType )
        return BMPtoPICT( aBmp, aPict );

    bool bResult = false;

    NSData* pData = [NSData dataWithBytesNoCopy: const_cast<sal_Int8*>(aBmp.getConstArray()) length: aBmp.getLength() freeWhenDone: 0];
    if( pData )
    {
        NSBitmapImageRep* pRep = [NSBitmapImageRep imageRepWithData: pData];
        if( pRep )
        {
            NSData* pOut = [pRep representationUsingType: eOutFormat properties: nil];
            if( pOut )
            {
                aPict.realloc( [pOut length] );
                [pOut getBytes: aPict.getArray() length: aPict.getLength()];
                bResult = (aPict.getLength() != 0);
            }
        }
    }

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
