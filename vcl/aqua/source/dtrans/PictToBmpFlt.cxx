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
#include <QuickTime/QuickTime.h>
#include <postmac.h>

#include <string.h>

#include "PictToBmpFlt.hxx"

bool ImageToBMP( com::sun::star::uno::Sequence<sal_Int8>& aPict,
                 com::sun::star::uno::Sequence<sal_Int8>& aBmp,
                 NSBitmapImageFileType eInFormat)
{
    (void) eInFormat; // Really not needed? Weird.

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
