/*************************************************************************
 *
 *  $RCSfile: salbmp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: pluby $ $Date: 2000-12-31 20:54:10 $
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

#define _SV_SALBMP_CXX

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif

// ==================================================================

SalBitmap::SalBitmap() :
        mhPixMap( 0 ),
        mnBitCount( 0 )
{
}

// ------------------------------------------------------------------

SalBitmap::~SalBitmap()
{
    Destroy();
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
{
    maSize = rSize;
    mnBitCount = nBitCount;
    return TRUE;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBitmap )
{
    maSize = rSalBitmap.maSize;
    mnBitCount = 1;
    return TRUE;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    maSize = rSalBmp.maSize;
    mnBitCount = rSalBmp.mnBitCount;
    return TRUE;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
{
    maSize = rSalBmp.maSize;
    mnBitCount = nNewBitCount;
    return TRUE;
}

// ------------------------------------------------------------------

void SalBitmap::Destroy()
{
    maSize = Size();
    mnBitCount = 0;
}

// ------------------------------------------------------------------

BitmapBuffer* SalBitmap::AcquireBuffer( BOOL bReadOnly )
{
    BitmapBuffer *pBuffer = new BitmapBuffer();

    // Stub code: we have not yet written any interfaces to native bitmaps.
    pBuffer->mnFormat = BMP_FORMAT_BOTTOM_UP | BMP_FORMAT_1BIT_MSB_PAL;
    pBuffer->mnWidth = maSize.Width();
    pBuffer->mnHeight = maSize.Height();
    pBuffer->mnScanlineSize = AlignedWidth4Bytes( pBuffer->mnWidth * mnBitCount );
    pBuffer->mnFormat |= BMP_FORMAT_16BIT_TC_MASK;
    pBuffer->maColorMask = ColorMask( 0x7b00, 0x03e0, 0x001f);
    pBuffer->mpBits = new BYTE[ pBuffer->mnScanlineSize * pBuffer->mnHeight ];

    return pBuffer;
}

// ------------------------------------------------------------------

void SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
    delete pBuffer;
}
