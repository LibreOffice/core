/*************************************************************************
 *
 *  $RCSfile: salgdilayout.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ssa $ $Date: 2002-07-11 07:31:51 $
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

#define _SV_OUTDEV_CXX
#include <tools/ref.hxx>
#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#else
#ifndef _SV_RMOUTDEV_HXX
#include <rmoutdev.hxx>
#endif
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_POLY_H
#include <poly.h>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_REGION_HXX
#include <region.hxx>
#endif
#ifndef _SV_REGION_H
#include <region.h>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_OUTDATA_HXX
#include <outdata.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <print.hxx>
#endif
#ifndef _SV_SALOTYPE_HXX
#include <salotype.hxx>
#endif
#ifndef _SV_OPENGL_HXX
#include <opengl.hxx>
#endif
#ifndef _VCL_IMPLNCVT_HXX
#include <implncvt.hxx>
#endif
#ifndef _SV_OUTDEV3D_HXX
#include <outdev3d.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _VCL_UNOWRAP_HXX
#include <unowrap.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif


// ----------------------------------------------------------------------------

SalGraphicsLayout::SalGraphicsLayout() : SalGraphics()
{
    static const char* pEnv = getenv("SAL_RTL_ENABLED" );
    if( pEnv )
        mnLayout = SAL_LAYOUT_BIDI_RTL;
    else
        mnLayout = 0;
}

SalGraphicsLayout::~SalGraphicsLayout()
{
}

// ----------------------------------------------------------------------------

void SalGraphicsLayout::mirror( long& x )
{
    long w = GetGraphicsWidth();
    if( w )
        x = w-1-x;
}

void SalGraphicsLayout::mirror( long& x, long& nWidth )
{
    long w = GetGraphicsWidth();
    if( w )
        x = w-nWidth-x;
}

BOOL SalGraphicsLayout::mirror( sal_uInt32 nPoints, const SalPoint *pPtAry, SalPoint *pPtAry2 )
{
    long w = GetGraphicsWidth();
    if( w )
    {
        sal_uInt32 i, j;
        for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
        {
            pPtAry2[j].mnX = w-1-pPtAry[i].mnX;
            pPtAry2[j].mnY = pPtAry[i].mnY;
        }
        return TRUE;
    }
    else
        return FALSE;
}

// ----------------------------------------------------------------------------

BOOL    SalGraphicsLayout::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX, nWidth );
    return SalGraphics::UnionClipRegion( nX, nY, nWidth, nHeight );
}

BOOL SalGraphicsLayout::GetGlyphBoundRect( long nGlyphIndex, Rectangle& rRect )
{
    // TODO: ???
    return SalGraphics::GetGlyphBoundRect( nGlyphIndex, rRect );
}
BOOL SalGraphicsLayout::GetGlyphOutline( long nGlyphIndex, PolyPolygon& rPolyPoly)
{
    // TODO: ???
    return SalGraphics::GetGlyphOutline( nGlyphIndex, rPolyPoly );
}
BOOL SalGraphicsLayout::GetLayoutOutline( const SalLayout& rStr, PolyPolygon& rPolyPoly )
{
    // TODO: ???
    return SalGraphics::GetLayoutOutline( rStr, rPolyPoly );
}
SalLayout* SalGraphicsLayout::LayoutText( const ImplLayoutArgs& rLayoutArgs )
{
    // TODO: ???
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        ImplLayoutArgs tmpArgs( rLayoutArgs );
        mirror( tmpArgs.maDrawPosition.X() );
        return SalGraphics::LayoutText( tmpArgs );
    }
    else
        return SalGraphics::LayoutText( rLayoutArgs );
}
void SalGraphicsLayout::DrawSalLayout( const SalLayout& rLayout)
{
    // TODO: ???
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        Point aPos = rLayout.GetDrawPosition();
        // TODO: check what should be done, with mirroring the pos seems
        // to be mirrored twice...
        //mirror( aPos.X() );
        ((SalLayout&) rLayout).SetDrawPosition( aPos );
    }
    SalGraphics::DrawSalLayout( rLayout );
}

// pre-CTL text methods, deprecated
/*
BOOL    SalGraphicsLayout::GetGlyphBoundRect( xub_Unicode cChar, long* pX, long* pY,
                                           long* pWidth, long* pHeight )
{
    return SalGraphics::GetGlyphBoundRect( cChar, pX, pY, pWidth, pHeight ); // ???
}
ULONG   SalGraphicsLayout::GetGlyphOutline( xub_Unicode cChar, USHORT** pPolySizes,
                                         SalPoint** ppPoints, BYTE** ppFlags )
{
    return SalGraphics::GetGlyphOutline( cChar, pPolySizes, ppPoints, ppFlags ); // ???
}
void    SalGraphicsLayout::DrawText( long nX, long nY,
                                  const xub_Unicode* pStr, xub_StrLen nLen )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX );
    SalGraphics::DrawText( nX, nY, pStr, nLen );
}
void    SalGraphicsLayout::DrawTextArray( long nX, long nY,
                                       const xub_Unicode* pStr, xub_StrLen nLen,
                                       const long* pDXAry )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX );
    SalGraphics::DrawTextArray( nX, nY, pStr, nLen, pDXAry );
}
*/

void    SalGraphicsLayout::DrawPixel( long nX, long nY )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX );
    SalGraphics::DrawPixel( nX, nY );
}
void    SalGraphicsLayout::DrawPixel( long nX, long nY, SalColor nSalColor )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX );
    SalGraphics::DrawPixel( nX, nY, nSalColor );
}
void    SalGraphicsLayout::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        mirror( nX1 );
        mirror( nX2 );
    }
    SalGraphics::DrawLine( nX1, nY1, nX2, nY2 );
}
void    SalGraphicsLayout::DrawRect( long nX, long nY, long nWidth, long nHeight )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX, nWidth );
    SalGraphics::DrawRect( nX, nY, nWidth, nHeight );
}
void    SalGraphicsLayout::DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        SalPoint* pPtAry2 = new SalPoint[nPoints];
        BOOL bCopied = mirror( nPoints, pPtAry, pPtAry2 );
        SalGraphics::DrawPolyLine( nPoints, bCopied ? pPtAry2 : pPtAry );
        delete [] pPtAry2;
    }
    else
        SalGraphics::DrawPolyLine( nPoints, pPtAry );
}
void    SalGraphicsLayout::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        SalPoint* pPtAry2 = new SalPoint[nPoints];
        BOOL bCopied = mirror( nPoints, pPtAry, pPtAry2 );
        SalGraphics::DrawPolygon( nPoints, bCopied ? pPtAry2 : pPtAry );
        delete [] pPtAry2;
    }
    else
        SalGraphics::DrawPolygon( nPoints, pPtAry );
}
void    SalGraphicsLayout::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry )
{
    DBG_ASSERT( !(mnLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolyPolygon - no mirroring implemented");
    SalGraphics::DrawPolyPolygon( nPoly, pPoints, pPtAry );
}
sal_Bool SalGraphicsLayout::DrawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    DBG_ASSERT( !(mnLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolyLineBezier - no mirroring implemented");
    return SalGraphics::DrawPolyLineBezier( nPoints, pPtAry, pFlgAry );
}
sal_Bool SalGraphicsLayout::DrawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    DBG_ASSERT( !(mnLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolygonBezier - no mirroring implemented");
    return SalGraphics::DrawPolygonBezier( nPoints, pPtAry, pFlgAry );
}
sal_Bool SalGraphicsLayout::DrawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints,
                                                   const SalPoint* const* pPtAry, const BYTE* const* pFlgAry )
{
    DBG_ASSERT( !(mnLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolyPolygonBezier - no mirroring implemented");
    return SalGraphics::DrawPolyPolygonBezier( nPoly, pPoints, pPtAry, pFlgAry );
}
void    SalGraphicsLayout::CopyArea( long nDestX, long nDestY,
                                  long nSrcX, long nSrcY,
                                  long nSrcWidth, long nSrcHeight,
                                  USHORT nFlags )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        mirror( nDestX, nSrcWidth );
        mirror( nSrcX, nSrcWidth );
    }
    SalGraphics::CopyArea( nDestX, nDestY,
                                  nSrcX, nSrcY,
                                  nSrcWidth, nSrcHeight,
                                  nFlags );
}
void    SalGraphicsLayout::CopyBits( const SalTwoRect* pPosAry,
                                  SalGraphics* pSrcGraphics )
{
    if( (mnLayout & SAL_LAYOUT_BIDI_RTL) || (pSrcGraphics && pSrcGraphics->GetLayout()) )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        if( pSrcGraphics && pSrcGraphics->GetLayout() )
            mirror( pPosAry2.mnSrcX, pPosAry2.mnSrcWidth );
        if( mnLayout & SAL_LAYOUT_BIDI_RTL )
            mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth );
        SalGraphics::CopyBits( &pPosAry2, pSrcGraphics );
    }
    else
        SalGraphics::CopyBits( pPosAry, pSrcGraphics );
}
void    SalGraphicsLayout::DrawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth );
        SalGraphics::DrawBitmap( &pPosAry2, rSalBitmap );
    }
    else
        SalGraphics::DrawBitmap( pPosAry, rSalBitmap );
}
void    SalGraphicsLayout::DrawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth );
        SalGraphics::DrawBitmap( &pPosAry2, rSalBitmap, nTransparentColor );
    }
    else
        SalGraphics::DrawBitmap( pPosAry, rSalBitmap, nTransparentColor );
}
void SalGraphicsLayout::DrawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth );
        SalGraphics::DrawBitmap( &pPosAry2, rSalBitmap, rTransparentBitmap );
    }
    else
        SalGraphics::DrawBitmap( pPosAry, rSalBitmap, rTransparentBitmap );
}
void    SalGraphicsLayout::DrawMask( const SalTwoRect* pPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth );
        SalGraphics::DrawMask( &pPosAry2, rSalBitmap, nMaskColor );
    }
    else
        SalGraphics::DrawMask( pPosAry, rSalBitmap, nMaskColor );
}
SalBitmap*  SalGraphicsLayout::GetBitmap( long nX, long nY, long nWidth, long nHeight )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX, nWidth );
    return SalGraphics::GetBitmap( nX, nY, nWidth, nHeight );
}
SalColor    SalGraphicsLayout::GetPixel( long nX, long nY )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX );
    return SalGraphics::GetPixel( nX, nY );
}
void    SalGraphicsLayout::Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX, nWidth );
    SalGraphics::Invert( nX, nY, nWidth, nHeight, nFlags );
}
void    SalGraphicsLayout::Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
    {
        SalPoint* pPtAry2 = new SalPoint[nPoints];
        BOOL bCopied = mirror( nPoints, pPtAry, pPtAry2 );
        SalGraphics::Invert( nPoints, bCopied ? pPtAry2 : pPtAry, nFlags );
        delete [] pPtAry2;
    }
    else
        SalGraphics::Invert( nPoints, pPtAry, nFlags );
}

BOOL    SalGraphicsLayout::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
    if( mnLayout & SAL_LAYOUT_BIDI_RTL )
        mirror( nX, nWidth );
    return SalGraphics::DrawEPS( nX, nY, nWidth, nHeight,  pPtr, nSize );
}

// ----------------------------------------------------------------------------
