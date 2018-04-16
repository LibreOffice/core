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

ImpBitmap::ImpBitmap()
    : mpSalBitmap(ImplGetSVData()->mpDefInst->CreateSalBitmap())
{
}

ImpBitmap::ImpBitmap(SalBitmap* pBitmap)
    : mpSalBitmap(pBitmap)
{
}

ImpBitmap::~ImpBitmap()
{
}

bool ImpBitmap::ImplIsEqual(const ImpBitmap& rBmp) const
{
    return (rBmp.GetSize() == GetSize() &&
        rBmp.GetBitCount() == GetBitCount() &&
        rBmp.GetChecksum() == GetChecksum());
}

void ImpBitmap::Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal)
{
    mpSalBitmap->Create( rSize, nBitCount, rPal );
}

bool ImpBitmap::Create( const ImpBitmap& rImpBitmap )
{
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap );
}

bool ImpBitmap::Create( const ImpBitmap& rImpBitmap, SalGraphics* pGraphics )
{
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap, pGraphics );
}

bool ImpBitmap::Create( const ImpBitmap& rImpBitmap, sal_uInt16 nNewBitCount )
{
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap, nNewBitCount );
}

Size ImpBitmap::GetSize() const
{
    return mpSalBitmap->GetSize();
}

sal_uInt16 ImpBitmap::GetBitCount() const
{
    sal_uInt16 nBitCount = mpSalBitmap->GetBitCount();
    return ( nBitCount <= 4 ) ? ( ( nBitCount <= 1 ) ? 1 : 4 ):
                                ( ( nBitCount <= 8 ) ? 8 : 24);
}

BitmapBuffer* ImpBitmap::AcquireBuffer( BitmapAccessMode nMode )
{
    return mpSalBitmap->AcquireBuffer( nMode );
}

void ImpBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
    mpSalBitmap->ReleaseBuffer( pBuffer, nMode );
}

BitmapChecksum ImpBitmap::GetChecksum() const
{
    BitmapChecksum aChecksum;
    mpSalBitmap->GetChecksum(aChecksum);
    return aChecksum;
}

void ImpBitmap::InvalidateChecksum()
{
    mpSalBitmap->InvalidateChecksum();
}

bool ImpBitmap::ScalingSupported() const
{
    return mpSalBitmap->ScalingSupported();
}

bool ImpBitmap::Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{
    return mpSalBitmap->Scale( rScaleX, rScaleY, nScaleFlag );
}

bool ImpBitmap::Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol )
{
    return mpSalBitmap->Replace( rSearchColor, rReplaceColor, nTol );
}

bool ImpBitmap::ConvertToGreyscale()
{
    return mpSalBitmap->ConvertToGreyscale();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
