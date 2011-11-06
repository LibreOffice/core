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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <tools/debug.hxx>

#include <vcl/bitmap.hxx>

#include <svdata.hxx>
#include <salinst.hxx>
#include <salbmp.hxx>
#include <impbmp.hxx>

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
