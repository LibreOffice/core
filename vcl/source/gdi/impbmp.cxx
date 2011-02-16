/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salbmp.hxx>
#include <tools/debug.hxx>
#include <vcl/impbmp.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/svdata.hxx>
#include <vcl/salinst.hxx>

// --------------
// - ImpBitmap  -
// --------------

ImpBitmap::ImpBitmap() :
            mnRefCount  ( 1UL ),
            mnChecksum  ( 0UL ),
            mpSalBitmap ( ImplGetSVData()->mpDefInst->CreateSalBitmap() ),
            maSourceSize( 0, 0 )
{
}

// -----------------------------------------------------------------------

ImpBitmap::~ImpBitmap()
{
    delete mpSalBitmap;
}

// -----------------------------------------------------------------------
void ImpBitmap::ImplSetSalBitmap( SalBitmap* pBitmap )
{
    delete mpSalBitmap, mpSalBitmap = pBitmap;
}

// -----------------------------------------------------------------------

sal_Bool ImpBitmap::ImplCreate( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal )
{
    maSourceSize = rSize;
    return mpSalBitmap->Create( rSize, nBitCount, rPal );
}

// -----------------------------------------------------------------------

sal_Bool ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap )
{
    maSourceSize = rImpBitmap.maSourceSize;
    mnChecksum = rImpBitmap.mnChecksum;
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap );
}

// -----------------------------------------------------------------------

sal_Bool ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap, SalGraphics* pGraphics )
{
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap, pGraphics );
}

// -----------------------------------------------------------------------

sal_Bool ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap, sal_uInt16 nNewBitCount )
{
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap, nNewBitCount );
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplDestroy()
{
    mpSalBitmap->Destroy();
}

// -----------------------------------------------------------------------

Size ImpBitmap::ImplGetSize() const
{
    return mpSalBitmap->GetSize();
}

// -----------------------------------------------------------------------

sal_uInt16 ImpBitmap::ImplGetBitCount() const
{
    sal_uInt16 nBitCount = mpSalBitmap->GetBitCount();
    return( ( nBitCount <= 1 ) ? 1 : ( nBitCount <= 4 ) ? 4 : ( nBitCount <= 8 ) ? 8 : 24 );
}

// -----------------------------------------------------------------------

BitmapBuffer* ImpBitmap::ImplAcquireBuffer( sal_Bool bReadOnly )
{
    return mpSalBitmap->AcquireBuffer( bReadOnly );
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplReleaseBuffer( BitmapBuffer* pBuffer, sal_Bool bReadOnly )
{
    mpSalBitmap->ReleaseBuffer( pBuffer, bReadOnly );

    if( !bReadOnly )
        mnChecksum = 0;
}
