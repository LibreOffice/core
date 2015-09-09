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

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#include <string.h>

#include "PictToBmpFlt.hxx"

bool ImageToPNG( css::uno::Sequence<sal_Int8>& rImgData,
                 css::uno::Sequence<sal_Int8>& rPngData,
                 NSBitmapImageFileType eInFormat)
{
    (void) eInFormat; // Really not needed? Weird.

    NSData* pData = [NSData dataWithBytesNoCopy: const_cast<sal_Int8 *>(rImgData.getConstArray()) length: rImgData.getLength() freeWhenDone: 0];
    if( !pData)
        return false;

    NSBitmapImageRep* pRep =[NSBitmapImageRep imageRepWithData: pData];
    if( !pRep)
        return false;

    NSData* pOut = [pRep representationUsingType: NSPNGFileType properties: [ NSDictionary dictionary ]];
    if( !pOut)
        return false;

    const size_t nPngSize = [pOut length];
    rPngData.realloc( nPngSize);
    [pOut getBytes: rPngData.getArray() length: nPngSize];
    return (nPngSize > 0);
}

bool PNGToImage( css::uno::Sequence<sal_Int8>& rPngData,
                 css::uno::Sequence<sal_Int8>& rImgData,
                 NSBitmapImageFileType eOutFormat
                )
{
    NSData* pData = [NSData dataWithBytesNoCopy: const_cast<sal_Int8*>(rPngData.getConstArray()) length: rPngData.getLength() freeWhenDone: 0];
    if( !pData)
        return false;

    NSBitmapImageRep* pRep = [NSBitmapImageRep imageRepWithData: pData];
    if( !pRep)
        return false;

    NSData* pOut = [pRep representationUsingType: eOutFormat properties: [ NSDictionary dictionary ]];
    if( !pOut)
        return false;

    const size_t nImgSize = [pOut length];
    rImgData.realloc( nImgSize);
    [pOut getBytes: rImgData.getArray() length: nImgSize];
    return (nImgSize > 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
