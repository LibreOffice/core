/*************************************************************************
 *
 *  $RCSfile: impbmp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_IMPBMP_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#else
#include <indbmp.hxx>
#ifndef _SV_RMBITMAP_HXX
#include <rmbitmap.hxx>
#endif
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <impbmp.hxx>
#include <bitmap.hxx>

// --------------
// - ImpBitmap  -
// --------------

ImpBitmap::ImpBitmap() :
            mnRefCount  ( 1UL ),
            mnChecksum  ( 0UL ),
            mpRMBitmap  ( NULL ),
#ifndef REMOTE_APPSERVER
            mpSalBitmap ( new SalBitmap )
#else
            mpSalBitmap ( new ImplServerBitmap )
#endif
{
}

// -----------------------------------------------------------------------

ImpBitmap::~ImpBitmap()
{
#ifdef REMOTE_APPSERVER
    ImplReleaseRemoteBmp();
#endif
    delete mpSalBitmap;
}

// -----------------------------------------------------------------------
#ifndef REMOTE_APPSERVER
void ImpBitmap::ImplSetSalBitmap( SalBitmap* pBitmap )
#else
void ImpBitmap::ImplSetSalBitmap( ImplServerBitmap* pBitmap )
#endif
{
    delete mpSalBitmap, mpSalBitmap = pBitmap;
}

// -----------------------------------------------------------------------

BOOL ImpBitmap::ImplCreate( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
{
    return mpSalBitmap->Create( rSize, nBitCount, rPal );
}

// -----------------------------------------------------------------------

BOOL ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap )
{
    mnChecksum = rImpBitmap.mnChecksum;
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap );
}

// -----------------------------------------------------------------------

BOOL ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap, SalGraphics* pGraphics )
{
    return mpSalBitmap->Create( *rImpBitmap.mpSalBitmap, pGraphics );
}

// -----------------------------------------------------------------------

BOOL ImpBitmap::ImplCreate( const ImpBitmap& rImpBitmap, USHORT nNewBitCount )
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
#ifdef REMOTE_APPSERVER
    if( ImplIsGetPrepared() )
        return mpRMBitmap->GetSize();
    else
#endif
        return mpSalBitmap->GetSize();
}

// -----------------------------------------------------------------------

USHORT ImpBitmap::ImplGetBitCount() const
{
    USHORT nBitCount;

#ifdef REMOTE_APPSERVER
    if( ImplIsGetPrepared() )
        nBitCount = mpRMBitmap->GetBitCount();
    else
#endif
        nBitCount = mpSalBitmap->GetBitCount();

    return( ( nBitCount <= 1 ) ? 1 : ( nBitCount <= 4 ) ? 4 : ( nBitCount <= 8 ) ? 8 : 24 );
}

// -----------------------------------------------------------------------

BitmapBuffer* ImpBitmap::ImplAcquireBuffer( BOOL bReadOnly )
{
    return mpSalBitmap->AcquireBuffer( bReadOnly );
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
    mpSalBitmap->ReleaseBuffer( pBuffer, bReadOnly );

    if( !bReadOnly )
        mnChecksum = 0;
}

#ifdef REMOTE_APPSERVER

RMBitmap* ImpBitmap::ImplGetRemoteBmp()
{
    return mpRMBitmap;
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplCreateRemoteBmp( const Bitmap& rBitmap )
{
    DBG_ASSERT( !mpRMBitmap, "ImplCreateRemoteBmp( Bitmap& rBitmap )???" );

    mpRMBitmap = new RMBitmap( &(Bitmap&) rBitmap );
    mpRMBitmap->Create();
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplCreateRemoteBmp( const Bitmap& rBitmap,
                                     OutputDevice* pOut,
                                     const Point& rPt, const Size& rSz )
{
    DBG_ASSERT( !mpRMBitmap, "ImplCreateRemoteBmp( Bitmap& rBitmap )???" );

    mpRMBitmap = new RMBitmap( &(Bitmap&) rBitmap );
    mpRMBitmap->CreateGet( pOut, rPt, rSz );
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplReleaseRemoteBmp()
{
    delete mpRMBitmap;
    mpRMBitmap = NULL;
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplDrawRemoteBmp( OutputDevice* pOut,
                            const Point& rSrcPt, const Size& rSrcSz,
                            const Point& rDestPt, const Size& rDestSz )
{
    if( mpRMBitmap )
        mpRMBitmap->Draw( pOut, rSrcPt, rSrcSz, rDestPt, rDestSz );
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplDrawRemoteBmpEx( OutputDevice* pOut,
                            const Point& rSrcPt, const Size& rSrcSz,
                            const Point& rDestPt, const Size& rDestSz,
                            const Bitmap& rMask )
{
    if( mpRMBitmap )
        mpRMBitmap->DrawEx( pOut, rSrcPt, rSrcSz, rDestPt, rDestSz, rMask );
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplDrawRemoteBmpAlpha( OutputDevice* pOut,
                            const Point& rSrcPt, const Size& rSrcSz,
                            const Point& rDestPt, const Size& rDestSz,
                            const AlphaMask& rAlpha )
{
    if( mpRMBitmap )
        mpRMBitmap->DrawAlpha( pOut, rSrcPt, rSrcSz, rDestPt, rDestSz, rAlpha );
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplDrawRemoteBmpMask( OutputDevice* pOut,
                            const Point& rSrcPt, const Size& rSrcSz,
                            const Point& rDestPt, const Size& rDestSz,
                            const Color& rColor )
{
    if( mpRMBitmap )
        mpRMBitmap->DrawMask( pOut, rSrcPt, rSrcSz, rDestPt, rDestSz, rColor );
}

// -----------------------------------------------------------------------

BOOL ImpBitmap::ImplIsGetPrepared() const
{
    return( mpRMBitmap ? mpRMBitmap->IsGetPrepared() : FALSE );
}

// -----------------------------------------------------------------------

void ImpBitmap::ImplResolveGet()
{
    if( mpRMBitmap )
    {
        Bitmap aBmp;
        mpRMBitmap->Get( aBmp );
        ImpBitmap* pGetImpBmp = aBmp.ImplGetImpBitmap();

        if( pGetImpBmp )
        {
            // wir nehmen der gegetteten Bitmap einfach
            // die SalBitmap weg; Null-Setzen nicht vergessen,
            // da die Bitmap die SalBitmap sonst abraeumt
            delete mpSalBitmap;
            mpSalBitmap = pGetImpBmp->mpSalBitmap;
            pGetImpBmp->mpSalBitmap = NULL;
        }
    }
}

#endif
