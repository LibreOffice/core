/*************************************************************************
 *
 *  $RCSfile: gcach_vdev.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hdu $ $Date: 2000-11-22 12:44:51 $
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

#include <gcach_vdev.hxx>

#include <svapp.hxx>
#include <bitmap.hxx>
#include <outfont.hxx>
#include <virdev.hxx>
#include <metric.hxx>

// =======================================================================
// VirtDevServerFont
// =======================================================================

// -----------------------------------------------------------------------

long VirtDevServerFont::FetchFontList( ImplDevFontList* pToAdd )
{
#if 0
    // TODO: add fonts on server but not on client to the list
    long nCount = 0;
#else
    // TODO: get fonts on server but not on client,
    // problem is that currently there is no serverside virtual device...
    VirtualDevice vdev( 1 );
    long nCount = vdev.GetDevFontCount();

    for( int i = 0; i < nCount; ++ i)
    {
        const FontInfo aFontInfo = vdev.GetDevFont( i );

        ImplFontData rData;
        rData.mpNext        = NULL;
        rData.mpSysData     = SERVERFONT_MAGIC;

        rData.maName        = aFontInfo.GetName();
        rData.maStyleName   = aFontInfo.GetStyleName();
        rData.mnWidth       = aFontInfo.GetWidth();
        rData.mnHeight      = aFontInfo.GetHeight();
        rData.meFamily      = aFontInfo.GetFamily();
        rData.meCharSet     = aFontInfo.GetCharSet();
        rData.meScript      = SCRIPT_DONTKNOW;
        rData.mePitch       = aFontInfo.GetPitch();
        rData.meWidthType   = aFontInfo.GetWidthType();
        rData.meWeight      = aFontInfo.GetWeight();
        rData.meItalic      = aFontInfo.GetItalic();
        rData.meType        = aFontInfo.GetType();
        rData.meFamily      = aFontInfo.GetFamily();

        rData.mnVerticalOrientation = 0;    // TODO: where to get this info?
        rData.mbOrientation = true;         // TODO: where to get this info?
        rData.mbDevice      = false;
        rData.mnQuality     = 0;            // prefer client-side fonts if available

        pToAdd->Add( new ImplFontData( rData ) );   // TODO: avoid copy if possible
    }
#endif

    return nCount;
}

// -----------------------------------------------------------------------

void VirtDevServerFont::ClearFontList()
{
    // TODO
}

// -----------------------------------------------------------------------

VirtDevServerFont* VirtDevServerFont::CreateFont( const ImplFontSelectData& rFSD )
{
    VirtDevServerFont* pServerFont = NULL;
    // TODO: search list of VirtDevServerFonts, return NULL if not found
    // pServerFont = new VirtDevServerFont( rFSD );
    return pServerFont;
}

// -----------------------------------------------------------------------

VirtDevServerFont::VirtDevServerFont( const ImplFontSelectData& rFSD )
:   ServerFont( rFSD)
{}

// -----------------------------------------------------------------------

void VirtDevServerFont::FetchFontMetric( ImplFontMetricData& rTo, long& rFactor ) const
{
    const ImplFontSelectData& aFSD = GetFontSelData();

    Font aFont;
    aFont.SetName       ( aFSD.maName );
    aFont.SetStyleName  ( aFSD.maStyleName );
    aFont.SetHeight     ( aFSD.mnHeight );
    aFont.SetWidth      ( aFSD.mnWidth );
    aFont.SetOrientation( aFSD.mnOrientation );
    aFont.SetVertical   ( GetFontSelData().mbVertical );

    VirtualDevice vdev( 1 );
    FontMetric aMetric( vdev.GetFontMetric( aFont ) );

    rFactor = 0x100;

    rTo.mnAscent        = aMetric.GetAscent();
    rTo.mnDescent       = aMetric.GetDescent();
    rTo.mnLeading       = aMetric.GetLeading();
    rTo.mnSlant         = aMetric.GetSlant();
    rTo.meType          = aMetric.GetType();
    rTo.mnFirstChar     = 0x0020;   // TODO: where to get this info?
    rTo.mnLastChar      = 0xFFFE;   // TODO: where to get this info?

    rTo.mnWidth         = aFSD.mnWidth;
    rTo.maName          = aFSD.maName;
    rTo.maStyleName     = aFSD.maStyleName;
    rTo.mnOrientation   = aFSD.mnOrientation;
    rTo.meFamily        = aFSD.meFamily;
    rTo.meCharSet       = aFSD.meCharSet;
    rTo.meWeight        = aFSD.meWeight;
    rTo.meItalic        = aFSD.meItalic;
    rTo.mePitch         = aFSD.mePitch;
    rTo.mbDevice        = FALSE;
}

// -----------------------------------------------------------------------

int VirtDevServerFont::GetGlyphIndex( sal_Unicode aChar ) const
{
    return aChar;
}

// -----------------------------------------------------------------------

void VirtDevServerFont::SetGlyphData( int nGlyphIndex, bool bWithBitmap, GlyphData& rGD ) const
{
    Font aFont;
    aFont.SetName       ( GetFontSelData().maName );
    aFont.SetStyleName  ( GetFontSelData().maStyleName );
    aFont.SetHeight     ( GetFontSelData().mnHeight );
    aFont.SetWidth      ( GetFontSelData().mnWidth );
    aFont.SetOrientation( GetFontSelData().mnOrientation );
    aFont.SetVertical   ( GetFontSelData().mbVertical );

    VirtualDevice vdev( 1 );
    vdev.SetFont( aFont );

    // get glyph metrics
    long nCharWidth;
    vdev.GetCharWidth( nGlyphIndex, nGlyphIndex, &nCharWidth );
    rGD.SetCharWidth( nCharWidth );

    const Rectangle aRect = vdev.GetTextRect( aRect, nGlyphIndex );
    rGD.SetOffset( aRect.Top(), aRect.Left() );
    rGD.SetDelta( vdev.GetTextWidth( nGlyphIndex ), 0 );
    const Size aSize( aRect.GetSize() );
    rGD.SetSize( aSize );

    if( bWithBitmap && !rGD.GetBitmap() )
    {
        // draw bitmap
        vdev.SetOutputSizePixel( aSize, TRUE );
        vdev.DrawText( Point(0,0)-rGD.GetMetric().GetOffset(), nGlyphIndex );

        // create new glyph item
        const Bitmap& rBitmap = vdev.GetBitmap( Point(0,0), aSize );
        rGD.SetBitmap( new Bitmap( rBitmap ) );
    }
}

// -----------------------------------------------------------------------

ULONG VirtDevServerFont::GetKernPairs( ImplKernPairData** ppImplKernPairs ) const
{
    Font aFont;
    aFont.SetName       ( GetFontSelData().maName );
    aFont.SetStyleName  ( GetFontSelData().maStyleName );
    aFont.SetHeight     ( GetFontSelData().mnHeight );
    aFont.SetWidth      ( GetFontSelData().mnWidth );
    aFont.SetOrientation( GetFontSelData().mnOrientation );
    aFont.SetVertical   ( GetFontSelData().mbVertical );

    VirtualDevice vdev( 1 );
    vdev.SetFont( aFont );

    ULONG nPairs = vdev.GetKerningPairCount();
    if( nPairs > 0 )
    {
        KerningPair* const pKernPairs = new KerningPair[ nPairs ];
        vdev.GetKerningPairs( nPairs, pKernPairs );

        *ppImplKernPairs = new ImplKernPairData[ nPairs ];
        ImplKernPairData* pTo = *ppImplKernPairs;
        KerningPair* pFrom = pKernPairs;
        for ( ULONG n = 0; n < nPairs; n++ )
        {
            pTo->mnChar1    = pFrom->nChar1;
            pTo->mnChar2    = pFrom->nChar2;
            pTo->mnKern     = pFrom->nKern;
            ++pFrom;
            ++pTo;
        }

        delete[] pKernPairs;
    }

    return nPairs;
}

// -----------------------------------------------------------------------

bool VirtDevServerFont::GetGlyphOutline( int nGlyphIndex, bool bOptimize, PolyPolygon& rPolyPoly ) const
{
    Font aFont;
    aFont.SetName       ( GetFontSelData().maName );
    aFont.SetStyleName  ( GetFontSelData().maStyleName );
    aFont.SetHeight     ( GetFontSelData().mnHeight );
    aFont.SetWidth      ( GetFontSelData().mnWidth );
    aFont.SetOrientation( GetFontSelData().mnOrientation );
    aFont.SetVertical   ( GetFontSelData().mbVertical );

    VirtualDevice vdev( 1 );
    vdev.SetFont( aFont );

    return vdev.GetGlyphOutline( nGlyphIndex, rPolyPoly, bOptimize);
}

// =======================================================================
