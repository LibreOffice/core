/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salgdilayout.cxx,v $
 * $Revision: 1.29 $
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

#include <tools/ref.hxx>
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salgdi.hxx>
#include <vcl/salframe.hxx>
#include <vcl/salvd.hxx>
#include <vcl/salprn.hxx>
#include <tools/debug.hxx>
#include <vcl/svdata.hxx>
#include <vcl/svapp.hxx>
#include <tools/poly.hxx>
#include <vcl/region.hxx>
#include <vcl/region.h>
#include <vcl/virdev.hxx>
#include <vcl/window.h>
#include <vcl/window.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/outdata.hxx>
#include <vcl/print.hxx>
#include <vcl/salotype.hxx>
#include <vcl/opengl.hxx>
#ifndef _VCL_IMPLNCVT_HXX
#include <implncvt.hxx>
#endif
#include <vcl/outdev3d.hxx>
#include <vcl/outdev.h>
#include <vcl/outdev.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/sallayout.hxx>

//#define USE_NEW_RTL_IMPLEMENTATION

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

bool SalGraphics::drawAlphaBitmap( const SalTwoRect&,
    const SalBitmap&, const SalBitmap& )
{
    return false;
}

// ----------------------------------------------------------------------------

void SalGraphics::mirror( long& x, const OutputDevice *pOutDev, bool bBack ) const
{
    long w;
    if( pOutDev && pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    if( w )
    {
        if( pOutDev && !pOutDev->IsRTLEnabled() )
        {
            OutputDevice *pOutDevRef = (OutputDevice*) pOutDev;
#ifdef USE_NEW_RTL_IMPLEMENTATION
            if( pOutDev->meOutDevType == OUTDEV_WINDOW )
                pOutDevRef = (OutputDevice*) ((Window *) pOutDev)->mpDummy4; // top of non-mirroring hierarchy
#endif

            // mirror this window back
            long devX = w-pOutDevRef->GetOutputWidthPixel()-pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
            if( bBack )
                x = x - devX + pOutDevRef->GetOutOffXPixel();
            else
                x = devX + (x - pOutDevRef->GetOutOffXPixel());
        }
        else
            x = w-1-x;
    }
}

void SalGraphics::mirror( long& x, long& nWidth, const OutputDevice *pOutDev, bool bBack ) const
{
    long w;
    if( pOutDev && pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    if( w )
    {
        if( pOutDev && !pOutDev->IsRTLEnabled() )
        {
            OutputDevice *pOutDevRef = (OutputDevice*) pOutDev;
#ifdef USE_NEW_RTL_IMPLEMENTATION
            if( pOutDev->meOutDevType == OUTDEV_WINDOW )
                pOutDevRef = (OutputDevice*) ((Window *) pOutDev)->mpDummy4; // top of non-mirroring hierarchy
#endif

            // mirror this window back
            long devX = w-pOutDevRef->GetOutputWidthPixel()-pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
            if( bBack )
                x = x - devX + pOutDevRef->GetOutOffXPixel();
            else
                x = devX + (x - pOutDevRef->GetOutOffXPixel());
        }
        else
            x = w-nWidth-x;

    }
}

BOOL SalGraphics::mirror( sal_uInt32 nPoints, const SalPoint *pPtAry, SalPoint *pPtAry2, const OutputDevice *pOutDev, bool bBack ) const
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
            OutputDevice *pOutDevRef = (OutputDevice*) pOutDev;
#ifdef USE_NEW_RTL_IMPLEMENTATION
            if( pOutDev->meOutDevType == OUTDEV_WINDOW )
                pOutDevRef = (OutputDevice*) ((Window *) pOutDev)->mpDummy4; // top of non-mirroring hierarchy
#endif

            // mirror this window back
            long devX = w-pOutDevRef->GetOutputWidthPixel()-pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
            if( bBack )
            {
                for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
                {
                    //long x = w-1-pPtAry[i].mnX;
                    //pPtAry2[j].mnX = devX + ( pOutDevRef->mnOutWidth - 1 - (x - devX) );
                    pPtAry2[j].mnX = pOutDevRef->GetOutOffXPixel() + (pPtAry[i].mnX - devX);
                    pPtAry2[j].mnY = pPtAry[i].mnY;
                }
            }
            else
            {
                for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
                {
                    //long x = w-1-pPtAry[i].mnX;
                    //pPtAry2[j].mnX = devX + ( pOutDevRef->mnOutWidth - 1 - (x - devX) );
                    pPtAry2[j].mnX = devX + (pPtAry[i].mnX - pOutDevRef->GetOutOffXPixel());
                    pPtAry2[j].mnY = pPtAry[i].mnY;
                }
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

void SalGraphics::mirror( Region& rRgn, const OutputDevice *pOutDev, bool bBack ) const
{
    // mirror the bounding rect and move Region by resulting offset
    Rectangle aRect( rRgn.GetBoundRect() );
    long nWidth = aRect.GetWidth();
    long x      = aRect.Left();
    long x_org = x;

    mirror( x, nWidth, pOutDev, bBack );
    rRgn.Move( x - x_org, 0 );
}

void SalGraphics::mirror( Rectangle& rRect, const OutputDevice *pOutDev, bool bBack ) const
{
    long nWidth = rRect.GetWidth();
    long x      = rRect.Left();
    long x_org = x;

    mirror( x, nWidth, pOutDev, bBack );
    rRect.Move( x - x_org, 0 );
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
void    SalGraphics::DrawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry, const OutputDevice *pOutDev )
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
            mirror( nPoints, pPtAry[i], pPtAry2[i], pOutDev );
        }

        drawPolyPolygon( nPoly, pPoints, (PCONSTSALPOINT*)pPtAry2 );

        for(i=0; i<nPoly; i++)
            delete [] pPtAry2[i];
        delete [] pPtAry2;
    }
    else
        drawPolyPolygon( nPoly, pPoints, pPtAry );
}
sal_Bool SalGraphics::DrawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry, const OutputDevice* )
{
    DBG_ASSERT( !(m_nLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolyLineBezier - no mirroring implemented");
    return drawPolyLineBezier( nPoints, pPtAry, pFlgAry );
}
sal_Bool SalGraphics::DrawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry, const OutputDevice* )
{
    DBG_ASSERT( !(m_nLayout & SAL_LAYOUT_BIDI_RTL), "DrawPolygonBezier - no mirroring implemented");
    return drawPolygonBezier( nPoints, pPtAry, pFlgAry );
}
sal_Bool SalGraphics::DrawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry, const BYTE* const* pFlgAry, const OutputDevice* )
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

BOOL SalGraphics::HitTestNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                                const Point& aPos, SalControlHandle& rControlHandle, BOOL& rIsInside, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        Point pt( aPos );
        Region rgn( rControlRegion );
        mirror( pt.X(), pOutDev );
        mirror( rgn, pOutDev );
        return hitTestNativeControl( nType, nPart, rgn, pt, rControlHandle, rIsInside );
    }
    else
        return hitTestNativeControl( nType, nPart, rControlRegion, aPos, rControlHandle, rIsInside );
}

void SalGraphics::mirror( ControlType nType, const ImplControlValue& rVal, const OutputDevice* pOutDev, bool bBack ) const
{
    if( rVal.getOptionalVal() )
    {
        switch( nType )
        {
            case CTRL_SCROLLBAR:
            {
                ScrollbarValue* pScVal = reinterpret_cast<ScrollbarValue*>(rVal.getOptionalVal());
                mirror(pScVal->maThumbRect,pOutDev,bBack);
                mirror(pScVal->maButton1Rect,pOutDev,bBack);
                mirror(pScVal->maButton2Rect,pOutDev,bBack);
            }
            break;
            case CTRL_SPINBOX:
            case CTRL_SPINBUTTONS:
            {
                SpinbuttonValue* pSpVal = reinterpret_cast<SpinbuttonValue*>(rVal.getOptionalVal());
                mirror(pSpVal->maUpperRect,pOutDev,bBack);
                mirror(pSpVal->maLowerRect,pOutDev,bBack);
            }
            break;
            case CTRL_TOOLBAR:
            {
                ToolbarValue* pTVal = reinterpret_cast<ToolbarValue*>(rVal.getOptionalVal());
                mirror(pTVal->maGripRect,pOutDev,bBack);
            }
            break;
        }
    }
}

BOOL SalGraphics::DrawNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                                ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                                const OUString& aCaption, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        Region rgn( rControlRegion );
        mirror( rgn, pOutDev );
        mirror( nType, aValue, pOutDev );
        BOOL bRet = drawNativeControl( nType, nPart, rgn, nState, aValue, rControlHandle, aCaption );
        mirror( nType, aValue, pOutDev, true );
        return bRet;
    }
    else
        return drawNativeControl( nType, nPart, rControlRegion, nState, aValue, rControlHandle, aCaption );
}

BOOL SalGraphics::DrawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                                ControlState nState, const ImplControlValue& aValue,
                                                SalControlHandle& rControlHandle, const OUString& aCaption, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        Region rgn( rControlRegion );
        mirror( rgn, pOutDev );
        mirror( nType, aValue, pOutDev );
        BOOL bRet = drawNativeControlText( nType, nPart, rgn, nState, aValue, rControlHandle, aCaption );
        mirror( nType, aValue, pOutDev, true );
        return bRet;
    }
    else
        return drawNativeControlText( nType, nPart, rControlRegion, nState, aValue, rControlHandle, aCaption );
}

BOOL SalGraphics::GetNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, SalControlHandle& rControlHandle, const OUString& aCaption,
                                                Region &rNativeBoundingRegion, Region &rNativeContentRegion, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        Region rgn( rControlRegion );
        mirror( rgn, pOutDev );
        mirror( nType, aValue, pOutDev );
        if( getNativeControlRegion( nType, nPart, rgn, nState, aValue, rControlHandle, aCaption,
                                                rNativeBoundingRegion, rNativeContentRegion ) )
        {
            mirror( rNativeBoundingRegion, pOutDev, true );
            mirror( rNativeContentRegion, pOutDev, true );
            mirror( nType, aValue, pOutDev, true );
            return TRUE;
        }
        else
        {
            mirror( nType, aValue, pOutDev, true );
            return FALSE;
        }
    }
    else
        return getNativeControlRegion( nType, nPart, rControlRegion, nState, aValue, rControlHandle, aCaption,
                                                rNativeBoundingRegion, rNativeContentRegion );
}

bool SalGraphics::DrawAlphaBitmap( const SalTwoRect& rPosAry,
                                   const SalBitmap& rSourceBitmap,
                                   const SalBitmap& rAlphaBitmap,
                                   const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
    {
        SalTwoRect pPosAry2 = rPosAry;
        mirror( pPosAry2.mnDestX, pPosAry2.mnDestWidth, pOutDev );
        return drawAlphaBitmap( pPosAry2, rSourceBitmap, rAlphaBitmap );
    }
    else
        return drawAlphaBitmap( rPosAry, rSourceBitmap, rAlphaBitmap );
}

bool SalGraphics::DrawAlphaRect( long nX, long nY, long nWidth, long nHeight,
                                 sal_uInt8 nTransparency, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SAL_LAYOUT_BIDI_RTL) )
        mirror( nX, nWidth, pOutDev );

    return drawAlphaRect( nX, nY, nWidth, nHeight, nTransparency );
}

bool SalGraphics::filterText( const String&, String&, xub_StrLen, xub_StrLen&, xub_StrLen&, xub_StrLen& )
{
    return false;
}

void SalGraphics::AddDevFontSubstitute( OutputDevice* pOutDev,
                                        const String& rFontName,
                                        const String& rReplaceFontName,
                                        USHORT nFlags )
{
    pOutDev->ImplAddDevFontSubstitute( rFontName, rReplaceFontName, nFlags );
}

