/*************************************************************************
 *
 *  $RCSfile: salgdilayout.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:34:38 $
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
#ifndef _POLY_HXX
#include <tools/poly.hxx>
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

#define IS_NOTRTL_ENABLED() ( pOutDev && !pOutDev->IsRTLEnabled() )


// ----------------------------------------------------------------------------
// The only common SalFrame method
// ----------------------------------------------------------------------------

SalFrameGeometry SalFrame::GetGeometry()
{
    // mirror frame coordinates at parent
    SalFrame *pParent = GetParent();
    if( pParent && Application::GetSettings().GetLayoutRTL() )
    {
        SalFrameGeometry aGeom = maGeometry;
        int parent_x = aGeom.nX - pParent->maGeometry.nX;
        aGeom.nX = pParent->maGeometry.nX + pParent->maGeometry.nWidth - maGeometry.nWidth - parent_x;
        return aGeom;
    }
    else
        return maGeometry;
}

// ----------------------------------------------------------------------------

SalGraphics::SalGraphics()
        : m_nLayout( 0 )
{
    // read global RTL settings
    if( Application::GetSettings().GetLayoutRTL() )
        m_nLayout = SAL_LAYOUT_BIDI_RTL;
}

SalGraphics::~SalGraphics()
{
}

// ----------------------------------------------------------------------------

void SalGraphics::mirror( long& x, const OutputDevice *pOutDev )
{
    long w;
    if( pOutDev && pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    if( w )
    {
        x = w-1-x;

        if( pOutDev && !pOutDev->IsRTLEnabled() )
        {
            // mirror this window back
            long devX = w-pOutDev->GetOutputWidthPixel()-pOutDev->GetOutOffXPixel();   // re-mirrored mnOutOffX
            x = devX + ( pOutDev->GetOutputWidthPixel() - 1 - (x - devX) ) ;
        }
    }
}

void SalGraphics::mirror( long& x, long& nWidth, const OutputDevice *pOutDev )
{
    long w;
    if( pOutDev && pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    if( w )
    {
        x = w-nWidth-x;

        if( pOutDev && !pOutDev->IsRTLEnabled() )
        {
            // mirror this window back
            long devX = w-pOutDev->GetOutputWidthPixel()-pOutDev->GetOutOffXPixel();   // re-mirrored mnOutOffX
            x = devX + ( pOutDev->GetOutputWidthPixel() - nWidth - (x - devX) ) ;
        }
    }
}

BOOL SalGraphics::mirror( sal_uInt32 nPoints, const SalPoint *pPtAry, SalPoint *pPtAry2, const OutputDevice *pOutDev )
{
    long w;
    if( pOutDev && pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    if( w )
    {
        sal_uInt32 i, j;

        if( pOutDev && !pOutDev->IsRTLEnabled() )
        {
            // mirror this window back
            long devX = w-pOutDev->GetOutputWidthPixel()-pOutDev->GetOutOffXPixel();   // re-mirrored mnOutOffX
            for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
            {
                long x = w-1-pPtAry[i].mnX;
                pPtAry2[j].mnX = devX + ( pOutDev->GetOutputWidthPixel() - 1 - (x - devX) );
                pPtAry2[j].mnY = pPtAry[i].mnY;
            }
        }
        else
        {
            for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
            {
                pPtAry2[j].mnX = w-1-pPtAry[i].mnX;
                pPtAry2[j].mnY = pPtAry[i].mnY;
            }
        }
        return TRUE;
    }
    else
        return FALSE;
}

// ----------------------------------------------------------------------------

BOOL    SalGraphics::UnionClipRegion( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, nWidth, pOutDev );
    return unionClipRegion( nX, nY, nWidth, nHeight );
}

void    SalGraphics::DrawPixel( long nX, long nY, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, pOutDev );
    drawPixel( nX, nY );
}
void    SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, pOutDev );
    drawPixel( nX, nY, nSalColor );
}
void    SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        mirror( nX1, pOutDev );
        mirror( nX2, pOutDev );
    }
    drawLine( nX1, nY1, nX2, nY2 );
}
void    SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, nWidth, pOutDev );
    drawRect( nX, nY, nWidth, nHeight );
}
void    SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        SalPoint* pPtAry2 = new SalPoint[nPoints];
        BOOL bCopied = mirror( nPoints, pPtAry, pPtAry2, pOutDev );
        drawPolyLine( nPoints, bCopied ? pPtAry2 : pPtAry );
        delete [] pPtAry2;
    }
    else
        drawPolyLine( nPoints, pPtAry );
}
void    SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        SalPoint* pPtAry2 = new SalPoint[nPoints];
        BOOL bCopied = mirror( nPoints, pPtAry, pPtAry2, pOutDev );
        drawPolygon( nPoints, bCopied ? pPtAry2 : pPtAry );
        delete [] pPtAry2;
    }
    else
        drawPolygon( nPoints, pPtAry );
}
void    SalGraphics::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        // TODO: optimize, reduce new/delete calls
        SalPoint **pPtAry2 = new SalPoint*[nPoly];
        ULONG i;
        for(i=0; i<nPoly; i++)
        {
            ULONG nPoints = pPoints[i];
            pPtAry2[i] = new SalPoint[ nPoints ];
            BOOL bCopied = mirror( nPoints, pPtAry[i], pPtAry2[i], pOutDev );
        }

        drawPolyPolygon( nPoly, pPoints, (PCONSTSALPOINT*)pPtAry2 );

        for(i=0; i<nPoly; i++)
            delete [] pPtAry2[i];
        delete [] pPtAry2;
    }
    else
        drawPolyPolygon( nPoly, pPoints, pPtAry );
}
sal_Bool SalGraphics::DrawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry, const OutputDevice *pOutDev )
{
    DBG_ASSERT( !(m_nLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolyLineBezier - no mirroring implemented");
    return drawPolyLineBezier( nPoints, pPtAry, pFlgAry );
}
sal_Bool SalGraphics::DrawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry, const OutputDevice *pOutDev )
{
    DBG_ASSERT( !(m_nLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolygonBezier - no mirroring implemented");
    return drawPolygonBezier( nPoints, pPtAry, pFlgAry );
}
sal_Bool SalGraphics::DrawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints,
                                                   const SalPoint* const* pPtAry, const BYTE* const* pFlgAry, const OutputDevice *pOutDev )
{
    DBG_ASSERT( !(m_nLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolyPolygonBezier - no mirroring implemented");
    return drawPolyPolygonBezier( nPoly, pPoints, pPtAry, pFlgAry );
}
void    SalGraphics::CopyArea( long nDestX, long nDestY,
                               long nSrcX, long nSrcY,
                               long nSrcWidth, long nSrcHeight,
                               USHORT nFlags, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        mirror( nDestX, nSrcWidth, pOutDev );
        mirror( nSrcX, nSrcWidth, pOutDev );
    }
    copyArea( nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, nFlags );
}
void    SalGraphics::CopyBits( const SalTwoRect* pPosAry,
                               SalGraphics* pSrcGraphics, const OutputDevice *pOutDev, const OutputDevice *pSrcOutDev )
{
    if( ( (m_nLayout & SAL_LAYOUT_BIDI_RTL) ) ||
        (pSrcGraphics && (pSrcGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) ) )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        if( pSrcGraphics && (pSrcGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) )
            mirror( pPosAry2.mnSrcX, pPosAry2.mnSrcWidth, pSrcOutDev );
        if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
            mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth, pOutDev );
        copyBits( &pPosAry2, pSrcGraphics );
    }
    else
        copyBits( pPosAry, pSrcGraphics );
}
void    SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth, pOutDev );
        drawBitmap( &pPosAry2, rSalBitmap );
    }
    else
        drawBitmap( pPosAry, rSalBitmap );
}
void    SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth, pOutDev );
        drawBitmap( &pPosAry2, rSalBitmap, nTransparentColor );
    }
    else
        drawBitmap( pPosAry, rSalBitmap, nTransparentColor );
}
void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransparentBitmap, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth, pOutDev );
        drawBitmap( &pPosAry2, rSalBitmap, rTransparentBitmap );
    }
    else
        drawBitmap( pPosAry, rSalBitmap, rTransparentBitmap );
}
void    SalGraphics::DrawMask( const SalTwoRect* pPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        SalTwoRect pPosAry2 = *pPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth, pOutDev );
        drawMask( &pPosAry2, rSalBitmap, nMaskColor );
    }
    else
        drawMask( pPosAry, rSalBitmap, nMaskColor );
}
SalBitmap*  SalGraphics::GetBitmap( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, nWidth, pOutDev );
    return getBitmap( nX, nY, nWidth, nHeight );
}
SalColor    SalGraphics::GetPixel( long nX, long nY, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, pOutDev );
    return getPixel( nX, nY );
}
void    SalGraphics::Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, nWidth, pOutDev );
    invert( nX, nY, nWidth, nHeight, nFlags );
}
void    SalGraphics::Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        SalPoint* pPtAry2 = new SalPoint[nPoints];
        BOOL bCopied = mirror( nPoints, pPtAry, pPtAry2, pOutDev );
        invert( nPoints, bCopied ? pPtAry2 : pPtAry, nFlags );
        delete [] pPtAry2;
    }
    else
        invert( nPoints, pPtAry, nFlags );
}

BOOL    SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, nWidth, pOutDev );
    return drawEPS( nX, nY, nWidth, nHeight,  pPtr, nSize );
}
