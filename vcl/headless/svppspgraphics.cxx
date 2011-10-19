/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "basegfx/vector/b2ivector.hxx"
#include "basegfx/point/b2ipoint.hxx"

#include "basebmp/color.hxx"

#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"
#include "vcl/bmpacc.hxx"
#include "vcl/svapp.hxx"
#include "vcl/sysdata.hxx"

#include "salprn.hxx"
#include "salbmp.hxx"
#include "impfont.hxx"
#include "outfont.hxx"
#include "fontsubset.hxx"
#include "generic/printergfx.hxx"
#include "headless/svppspgraphics.hxx"
#include "headless/svpbmp.hxx"
#include "generic/glyphcache.hxx"
#include "region.h"

using namespace psp;
using namespace basebmp;
using namespace basegfx;
using ::rtl::OUString;
using ::rtl::OString;

// FIXME: this is presumed un-necessary the generic/ BitmapBuffer code should work just as well - surely

#ifdef USE_SVP_BITMAP
// ----- Implementation of PrinterBmp by means of SalBitmap/BitmapBuffer ---------------

class SalPrinterBmp : public psp::PrinterBmp
{
    private:
    SalPrinterBmp ();

    BitmapDeviceSharedPtr       m_aBitmap;
    public:

                            SalPrinterBmp (const BitmapDeviceSharedPtr& rDevice);
        virtual             ~SalPrinterBmp ();
        virtual sal_uInt32  GetPaletteColor (sal_uInt32 nIdx) const;
        virtual sal_uInt32  GetPaletteEntryCount () const;
        virtual sal_uInt32  GetPixelRGB  (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt8   GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt8   GetPixelIdx  (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt32  GetWidth () const;
        virtual sal_uInt32  GetHeight() const;
        virtual sal_uInt32  GetDepth () const;

        static sal_uInt32 getRGBFromColor( const basebmp::Color& rCol )
        {
            return    ((rCol.getBlue())          & 0x000000ff)
                    | ((rCol.getGreen() <<  8) & 0x0000ff00)
                    | ((rCol.getRed()   << 16) & 0x00ff0000);
        }
};

SalPrinterBmp::SalPrinterBmp(const BitmapDeviceSharedPtr& rDevice) :
    m_aBitmap( rDevice )
{
}

SalPrinterBmp::~SalPrinterBmp ()
{
}

sal_uInt32
SalPrinterBmp::GetWidth () const
{
    return m_aBitmap.get() ? m_aBitmap->getSize().getX() : 0;
}

sal_uInt32
SalPrinterBmp::GetHeight () const
{
    return m_aBitmap.get() ? m_aBitmap->getSize().getY() : 0;
}

sal_uInt32
SalPrinterBmp::GetDepth () const
{
    return m_aBitmap.get() ?
           SvpElement::getBitCountFromScanlineFormat( m_aBitmap->getScanlineFormat() )
           : 0;
}


sal_uInt32
SalPrinterBmp::GetPaletteEntryCount () const
{
    return m_aBitmap.get() ? m_aBitmap->getPaletteEntryCount() : 0;
}

sal_uInt32
SalPrinterBmp::GetPaletteColor (sal_uInt32 nIdx) const
{
    sal_uInt32 nCol = 0;
    if( m_aBitmap.get() && nIdx < static_cast<sal_uInt32>(m_aBitmap->getPaletteEntryCount()) )
    {
        const basebmp::Color& rColor = (*m_aBitmap->getPalette().get())[ nIdx ];
        nCol = getRGBFromColor( rColor );
    }
    return nCol;
}

sal_uInt32
SalPrinterBmp::GetPixelRGB (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    sal_uInt32 nCol = 0;
    if( m_aBitmap.get() )
        nCol = getRGBFromColor( m_aBitmap->getPixel( B2IPoint( nColumn, nRow ) ) );
    return nCol;
}

sal_uInt8
SalPrinterBmp::GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    sal_uInt8 nGray = 0;
    if( m_aBitmap.get() )
    {
        // TODO: don't use tools color
        basebmp::Color aCol = m_aBitmap->getPixel( B2IPoint( nColumn, nRow ) );
        ::Color aColor( aCol.getRed(), aCol.getGreen(), aCol.getBlue() );
        nGray = aColor.GetLuminance();
    }
    return nGray;
}

sal_uInt8
SalPrinterBmp::GetPixelIdx (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    sal_uInt8 nIdx = 0;
    if( m_aBitmap.get() )
        nIdx = static_cast<sal_uInt8>(m_aBitmap->getPixelData( B2IPoint( nColumn, nRow ) ));
    return nIdx;
}

void SvpPspGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
    Rectangle aSrc (Point(pPosAry->mnSrcX, pPosAry->mnSrcY),
                    Size(pPosAry->mnSrcWidth, pPosAry->mnSrcHeight));
    Rectangle aDst (Point(pPosAry->mnDestX, pPosAry->mnDestY),
                    Size(pPosAry->mnDestWidth, pPosAry->mnDestHeight));

    const SvpSalBitmap* pBmp = dynamic_cast<const SvpSalBitmap*>(&rSalBitmap);
    if( pBmp )
    {
        SalPrinterBmp aBmp(pBmp->getBitmap());
        m_pPrinterGfx->DrawBitmap(aDst, aSrc, aBmp);
    }
}

#endif

SvpPspGraphics::~SvpPspGraphics()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
