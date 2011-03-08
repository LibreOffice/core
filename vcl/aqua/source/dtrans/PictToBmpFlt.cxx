/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OSXTransferable.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

bool PICTtoBMP(com::sun::star::uno::Sequence<sal_Int8>& aPict,
               com::sun::star::uno::Sequence<sal_Int8>& aBmp)
{

  bool result = false;

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
      rtl_copyMemory(aBmp.getArray(), ((sal_Int8*)*hBmp), sz);
      HUnlock(hBmp);

      result = true;
    }

  DisposeHandle(hPict);
  DisposeHandle(hBmp);
  CloseComponent(bmpExporter);

  return result;
}

bool BMPtoPICT(com::sun::star::uno::Sequence<sal_Int8>& aBmp,
               com::sun::star::uno::Sequence<sal_Int8>& aPict)
{
  bool result = false;

  Handle hBmp;
  ComponentInstance pictExporter;
  if ((PtrToHand(aBmp.getArray(), &hBmp, aBmp.getLength()) != noErr))
    {
      return result;
    }

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
      rtl_copyMemory(aPict.getArray(), ((sal_Int8*)*hPict), sz);
      HUnlock((Handle)hPict);

      // Release the data associated with the picture
      // Note: This function is deprecated in Mac OS X
      // 10.4.
      KillPicture(hPict);

      result = true;
    }

  DisposeHandle(hBmp);
  CloseComponent(pictExporter);

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
