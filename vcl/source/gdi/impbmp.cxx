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


#include <vcl/bitmap.hxx>

#include <svdata.hxx>
#include <salinst.hxx>
#include <salbmp.hxx>
#include <impbmp.hxx>

ImpBitmap::ImpBitmap() :
            mnRefCount  ( 1 ),
            mnChecksum  ( 0 ),
            mpSalBitmap ( ImplGetSVData()->mpDefInst->CreateSalBitmap() )
{
}

ImpBitmap::~ImpBitmap()
{
    delete mpSalBitmap;
}

void ImpBitmap::ImplSetSalBitmap( SalBitmap* pBitmap )
{
    delete mpSalBitmap, mpSalBitmap = pBitmap;
}

bool ImpBitmap::ImplCreate( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal )
{
    return mpSalBitmap->Create( rSize, nBitCount, rPal );
}

bool ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap )
{
    mnChecksum = rImpBitmap.mnChecksum;
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap );
}

bool ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap, SalGraphics* pGraphics )
{
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap, pGraphics );
}

bool ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap, sal_uInt16 nNewBitCount )
{
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap, nNewBitCount );
}

Size ImpBitmap::ImplGetSize() const
{
    return mpSalBitmap->GetSize();
}

sal_uInt16 ImpBitmap::ImplGetBitCount() const
{
    sal_uInt16 nBitCount = mpSalBitmap->GetBitCount();
    return( ( nBitCount <= 1 ) ? 1 : ( nBitCount <= 4 ) ? 4 : ( nBitCount <= 8 ) ? 8 : 24 );
}

BitmapBuffer* ImpBitmap::ImplAcquireBuffer( bool bReadOnly )
{
    return mpSalBitmap->AcquireBuffer( bReadOnly );
}

void ImpBitmap::ImplReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly )
{
    mpSalBitmap->ReleaseBuffer( pBuffer, bReadOnly );

    if( !bReadOnly )
        mnChecksum = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
