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

#include <string.h>
#include <svpm.h>

#define _SV_SALGDI_CXX
#include <tools/debug.hxx>
#include <saldata.hxx>
#include <salgdi.h>
#include <tools/debug.hxx>
#include <salframe.h>
#include <tools/poly.hxx>
#ifndef _RTL_STRINGBUF_HXX
#include <rtl/strbuf.hxx>
#endif
#include "vcl/region.h"

#ifndef __H_FT2LIB
#include <wingdi.h>
#include <ft2lib.h>
#endif

// -----------
// - Defines -
// -----------

// ClipRegions funktionieren immer noch nicht auf allen getesteten Druckern
#define SAL_PRINTER_CLIPPATH    1
// #define SAL_PRINTER_POLYPATH 1

// =======================================================================

void ImplInitSalGDI()
{
}

// -----------------------------------------------------------------------

void ImplFreeSalGDI()
{
    SalData*    pSalData = GetSalData();

    // delete icon cache
    SalIcon* pIcon = pSalData->mpFirstIcon;
    while( pIcon )
    {
        SalIcon* pTmp = pIcon->pNext;
        WinDestroyPointer( pIcon->hIcon );
        delete pIcon;
        pIcon = pTmp;
    }

}

// =======================================================================

void ImplSalInitGraphics( Os2SalGraphics* pData )
{
    GpiCreateLogColorTable( pData->mhPS, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );
}

// -----------------------------------------------------------------------

void ImplSalDeInitGraphics( Os2SalGraphics* pData )
{
}

// =======================================================================

Os2SalGraphics::Os2SalGraphics()
{
    for( int i = 0; i < MAX_FALLBACK; ++i )
    {
        mhFonts[ i ] = 0;
        mpOs2FontData[ i ]  = NULL;
        mpOs2FontEntry[ i ] = NULL;
    }

    mfFontScale = 1.0;

    mhPS            = 0;
    mhDC            = 0;
    mbLine              = FALSE;
    mbFill              = FALSE;
    mbXORMode           = FALSE;
    mnFontMetricCount   = 0;
    mpFontMetrics       = NULL;
    mpClipRectlAry      = NULL;

    mhDefFont           = 0;
    mpFontKernPairs     = NULL;
    mnFontKernPairCount = 0;
    mbFontKernInit      = FALSE;

}

// -----------------------------------------------------------------------

Os2SalGraphics::~Os2SalGraphics()
{
    Ft2DeleteSetId( mhPS, LCID_BASE);

    if ( mpFontMetrics )
        delete mpFontMetrics;

    if ( mpFontKernPairs )
        delete mpFontKernPairs;

}

// -----------------------------------------------------------------------

static SalColor ImplGetROPSalColor( SalROPColor nROPColor )
{
    SalColor nSalColor;

    switch( nROPColor )
    {
        case SAL_ROP_0:
            nSalColor = MAKE_SALCOLOR( 0, 0, 0 );
        break;

        case SAL_ROP_1:
        case SAL_ROP_INVERT:
            nSalColor = MAKE_SALCOLOR( 255, 255, 255 );
        break;
    }

    return nSalColor;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    // since OOo asks for DPI, I will query FONT_RES, which seems to be
    // more correct than _RESOLUTION fields (on my wide screen lcd)
    // and does not require conversion
    DevQueryCaps( mhDC, CAPS_HORIZONTAL_FONT_RES, 1, &rDPIX );
    DevQueryCaps( mhDC, CAPS_VERTICAL_FONT_RES, 1, &rDPIY );
}

// -----------------------------------------------------------------------

USHORT Os2SalGraphics::GetBitCount()
{
    LONG nBitCount;
    DevQueryCaps( mhDC, CAPS_COLOR_BITCOUNT, 1, &nBitCount );
    return (USHORT)nBitCount;
}

// -----------------------------------------------------------------------

long Os2SalGraphics::GetGraphicsWidth() const
{
    if( mhWnd )
    {
        Os2SalFrame* pFrame = (Os2SalFrame*)GetWindowPtr( mhWnd );
        if( pFrame )
        {
            if( pFrame->maGeometry.nWidth )
                return pFrame->maGeometry.nWidth;
            else
            {
                // TODO: perhaps not needed, maGeometry should always be up-to-date
                RECTL aRect;
                WinQueryWindowRect( mhWnd, &aRect );
                return aRect.xRight;
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::ResetClipRegion()
{
#ifdef SAL_PRINTER_CLIPPATH
    if ( mbPrinter )
        GpiSetClipPath( mhPS, 0, SCP_RESET );
    else
#endif
    {
        HRGN hOldRegion;

        GpiSetClipRegion( mhPS, NULL, &hOldRegion );
        if ( hOldRegion )
            GpiDestroyRegion( mhPS, hOldRegion );
    }
}

// -----------------------------------------------------------------------

bool Os2SalGraphics::setClipRegion( const Region& i_rClip )
{
    ULONG nCount = i_rClip.GetRectCount();

    mpClipRectlAry    = new RECTL[ nCount ];
    mnClipElementCount = 0;

    ImplRegionInfo aInfo;
    long nX, nY, nW, nH;
    bool bRegionRect = i_rClip.ImplGetFirstRect(aInfo, nX, nY, nW, nH );
    while( bRegionRect )
    {
        if ( nW && nH )
        {
            RECTL* pClipRect = &mpClipRectlAry[ mnClipElementCount ];
            pClipRect->xLeft   = nX;
            pClipRect->yTop    = mnHeight - nY;
            pClipRect->xRight  = nX + nW;
            pClipRect->yBottom = mnHeight - (nY + nH);
            mnClipElementCount++;
        }
        bRegionRect = i_rClip.ImplGetNextRect( aInfo, nX, nY, nW, nH );
    }
#ifdef SAL_PRINTER_CLIPPATH
    if ( mbPrinter )
    {
        GpiSetClipPath( mhPS, 0, SCP_RESET );
        GpiBeginPath( mhPS, 1L );

        for( int i = 0; i < mnClipElementCount; i++ )
        {
            POINTL aPt;
            RECTL* pClipRect = &mpClipRectlAry[ i ];

            aPt.x = pClipRect->xLeft;
            aPt.y = pClipRect->yTop-1;
            Ft2Move( mhPS, &aPt );

            aPt.x = pClipRect->xRight-1;
            aPt.y = pClipRect->yBottom;

            Ft2Box( mhPS, DRO_OUTLINE, &aPt, 0, 0 );
        }

        GpiEndPath( mhPS );
        GpiSetClipPath( mhPS, 1L, SCP_ALTERNATE | SCP_AND );
    }
    else
#endif
    {
        HRGN hClipRegion = GpiCreateRegion( mhPS,
                                            mnClipElementCount,
                                            mpClipRectlAry );
        HRGN hOldRegion;

        GpiSetClipRegion( mhPS, hClipRegion, &hOldRegion );
        if( hOldRegion )
            GpiDestroyRegion( mhPS, hOldRegion );
    }

    delete [] mpClipRectlAry;

    return true;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::SetLineColor()
{
    // don't draw line!
    mbLine = FALSE;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::SetLineColor( SalColor nSalColor )
{
    LINEBUNDLE lb;

    // set color
    lb.lColor = RGBCOLOR( SALCOLOR_RED( nSalColor ),
                          SALCOLOR_GREEN( nSalColor ),
                          SALCOLOR_BLUE( nSalColor ) );

    Ft2SetAttrs( mhPS,
                 PRIM_LINE,
                 LBB_COLOR,
                 0,
                 &lb );

    // draw line!
    mbLine = TRUE;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::SetFillColor()
{
    // don't fill area!
    mbFill = FALSE;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::SetFillColor( SalColor nSalColor )
{
    AREABUNDLE ab;

    // set color
    ab.lColor = RGBCOLOR( SALCOLOR_RED( nSalColor ),
                          SALCOLOR_GREEN( nSalColor ),
                          SALCOLOR_BLUE( nSalColor ) );

    Ft2SetAttrs( mhPS,
                 PRIM_AREA,
                 ABB_COLOR,
                 0,
                 &ab );

    // fill area!
    mbFill = TRUE;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::SetXORMode( bool bSet, bool )
{
    mbXORMode = bSet;
    LONG nMixMode = bSet ? FM_XOR : FM_OVERPAINT;

    // set mix mode for lines
    LINEBUNDLE lb;
    lb.usMixMode = nMixMode;
    Ft2SetAttrs( mhPS,
                 PRIM_LINE,
                 LBB_MIX_MODE,
                 0,
                 &lb );

    // set mix mode for areas
    AREABUNDLE ab;
    ab.usMixMode = nMixMode;
    Ft2SetAttrs( mhPS,
                 PRIM_AREA,
                 ABB_MIX_MODE,
                 0,
                 &ab );

    // set mix mode for text
    CHARBUNDLE cb;
    cb.usMixMode = nMixMode;
    Ft2SetAttrs( mhPS,
                 PRIM_CHAR,
                 CBB_MIX_MODE,
                 0,
                 &cb );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    SetLineColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    SetFillColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawPixel( long nX, long nY )
{
    POINTL aPt;

    aPt.x = nX;
    aPt.y = TY( nY );

    // set color
    Ft2SetPel( mhPS, &aPt );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    // save old color
    LINEBUNDLE oldLb;
    GpiQueryAttrs( mhPS,
                   PRIM_LINE,
                   LBB_COLOR,
                   &oldLb );

    // set new color
    LINEBUNDLE lb;
    lb.lColor = RGBCOLOR( SALCOLOR_RED( nSalColor ),
                          SALCOLOR_GREEN( nSalColor ),
                          SALCOLOR_BLUE( nSalColor ) );
    Ft2SetAttrs( mhPS,
                 PRIM_LINE,
                 LBB_COLOR,
                 0,
                 &lb );

    // set color of pixel
    POINTL aPt;
    aPt.x = nX;
    aPt.y = TY( nY );
    Ft2SetPel( mhPS, &aPt );

    // restore old color
    Ft2SetAttrs( mhPS,
                 PRIM_LINE,
                 LBB_COLOR,
                 0,
                 &oldLb );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    // OS2 zeichnet den Endpunkt mit
    POINTL aPt;
    aPt.x = nX1;
    aPt.y = TY( nY1 );
    Ft2Move( mhPS, &aPt );
    aPt.x = nX2;
    aPt.y = TY( nY2 );
    GpiLine( mhPS, &aPt );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    POINTL aPt;
    long lControl;

    if ( mbFill )
    {
        if ( mbLine )
            lControl = DRO_OUTLINEFILL;
        else
            lControl = DRO_FILL;
    }
    else
    {
        if ( mbLine )
            lControl = DRO_OUTLINE;
        else
            return;
    }

    aPt.x = nX;
    aPt.y = TY( nY );
    Ft2Move( mhPS, &aPt );
    aPt.x = nX + nWidth - 1;
    aPt.y = TY( nY + nHeight - 1 );
    Ft2Box( mhPS, lControl, &aPt, 0, 0 );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawPolyLine( ULONG nPoints, const SalPoint* pPtAry )
{
    // convert all points to sys orientation
    POINTL*             pOS2PtAry = new POINTL[ nPoints ];
    POINTL*             pTempOS2PtAry = pOS2PtAry;
    const SalPoint*     pTempPtAry = pPtAry;
    ULONG               nTempPoints = nPoints;
    long                nHeight = mnHeight - 1;

    while( nTempPoints-- )
    {
        (*pTempOS2PtAry).x = (*pTempPtAry).mnX;
        (*pTempOS2PtAry).y = nHeight - (*pTempPtAry).mnY;
        pTempOS2PtAry++;
        pTempPtAry++;
    }

    Ft2Move( mhPS, pOS2PtAry );
    GpiPolyLine( mhPS, nPoints, pOS2PtAry );
    delete [] pOS2PtAry;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    PM_POLYGON aPolygon;

    // create polygon
    aPolygon.aPointl = new POINTL[ nPoints ];
    aPolygon.ulPoints = nPoints;

    // convert all points to sys orientation
    POINTL*             pTempOS2PtAry = aPolygon.aPointl;
    const SalPoint*     pTempPtAry = pPtAry;
    ULONG               nTempPoints = nPoints;
    long                nHeight = mnHeight - 1;

    while( nTempPoints-- )
    {
        (*pTempOS2PtAry).x = (*pTempPtAry).mnX;
        (*pTempOS2PtAry).y = nHeight - (*pTempPtAry).mnY;
        pTempOS2PtAry++;
        pTempPtAry++;
    }

    // Innenleben zeichnen
    if ( mbFill )
    {
#ifdef SAL_PRINTER_POLYPATH
        if ( mbPrinter )
        {
            Ft2BeginPath( mhPS, 1 );
            Ft2Move( mhPS, aPolygon.aPointl );
            Ft2PolyLine( mhPS, aPolygon.ulPoints, aPolygon.aPointl );
            Ft2EndPath( mhPS );
            Ft2FillPath( mhPS, 1, 0 );

            if ( mbLine )
            {
                Ft2Move( mhPS, aPolygon.aPointl );
                Ft2PolyLine( mhPS, aPolygon.ulPoints, aPolygon.aPointl );
            }
        }
        else
#endif
        {
            ULONG nOptions = POLYGON_ALTERNATE;

            if ( mbLine )
                nOptions |= POLYGON_BOUNDARY;
            else
                nOptions |= POLYGON_NOBOUNDARY;

            Ft2Move( mhPS, aPolygon.aPointl );
            GpiPolygons( mhPS, 1, &aPolygon, nOptions, POLYGON_EXCL );
        }
    }
    else
    {
        if ( mbLine )
        {
            Ft2Move( mhPS, aPolygon.aPointl );
            GpiPolyLine( mhPS, nPoints, aPolygon.aPointl );
        }
    }

    delete [] aPolygon.aPointl;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
                                   PCONSTSALPOINT* pPtAry )
{
    ULONG       i;
    long        nHeight = mnHeight - 1;
    PM_POLYGON* aPolygonAry = new PM_POLYGON[ nPoly ];

    for( i = 0; i < nPoly; i++ )
    {
        const SalPoint * pTempPtAry = (const SalPoint*)pPtAry[ i ];

        // create polygon
        ULONG nTempPoints = pPoints[ i ];
        POINTL * pTempOS2PtAry = new POINTL[ nTempPoints ];

        // convert all points to sys orientation
        aPolygonAry[ i ].ulPoints = nTempPoints;
        aPolygonAry[ i ].aPointl = pTempOS2PtAry;

        while( nTempPoints-- )
        {
            (*pTempOS2PtAry).x = (*pTempPtAry).mnX;
            (*pTempOS2PtAry).y = nHeight - (*pTempPtAry).mnY;
            pTempOS2PtAry++;
            pTempPtAry++;
        }
    }

    // Innenleben zeichnen
    if ( mbFill )
    {
#ifdef SAL_PRINTER_POLYPATH
        if ( mbPrinter )
        {
            Ft2BeginPath( mhPS, 1 );
            for ( i = 0; i < nPoly; i++ )
            {
                Ft2Move( mhPS, aPolygonAry[i].aPointl );
                Ft2PolyLine( mhPS, aPolygonAry[i].ulPoints, aPolygonAry[i].aPointl );
            }
            Ft2EndPath( mhPS );
            Ft2FillPath( mhPS, 1, 0 );
        }
        else
#endif
        {
            ULONG nOptions = POLYGON_ALTERNATE;

            if ( mbLine )
                nOptions |= POLYGON_BOUNDARY;
            else
                nOptions |= POLYGON_NOBOUNDARY;

            Ft2Move( mhPS, aPolygonAry[ 0 ].aPointl );
            GpiPolygons( mhPS, nPoly, aPolygonAry, nOptions, POLYGON_EXCL );
        }
    }
    else
    {
        if ( mbLine )
        {
            for( i = 0; i < nPoly; i++ )
            {
                Ft2Move( mhPS, aPolygonAry[ i ].aPointl );
                GpiPolyLine( mhPS, aPolygonAry[ i ].ulPoints, aPolygonAry[ i ].aPointl );
            }
        }
    }

    // cleanup
    for( i = 0; i < nPoly; i++ )
        delete [] aPolygonAry[ i ].aPointl;
    delete [] aPolygonAry;
}

// -----------------------------------------------------------------------

bool Os2SalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double /*fTransparency*/ )
{
    // TODO: implement and advertise OutDevSupport_B2DDraw support
    return false;
}

// -----------------------------------------------------------------------

bool Os2SalGraphics::drawPolyLine(
    const basegfx::B2DPolygon& /*rPolygon*/,
    double /*fTransparency*/,
    const basegfx::B2DVector& /*rLineWidths*/,
    basegfx::B2DLineJoin /*eLineJoin*/)
{
    // TODO: implement
    return false;
}

// -----------------------------------------------------------------------

sal_Bool Os2SalGraphics::drawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Os2SalGraphics::drawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Os2SalGraphics::drawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints,
                                             const SalPoint* const* pPtAry, const BYTE* const* pFlgAry )
{
    return sal_False;
}

// =======================================================================

// MAXIMUM BUFSIZE EQ 0xFFFF
#define POSTSCRIPT_BUFSIZE          0x4000
// we only try to get the BoundingBox in the first 4096 bytes
#define POSTSCRIPT_BOUNDINGSEARCH   0x1000

static BYTE* ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize )
{
    while ( nComp-- >= nSize )
    {
        ULONG   i;
        for ( i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
                break;
        }
        if ( i == nSize )
            return pSource;
        pSource++;
    }
    return NULL;
}


static BOOL ImplGetBoundingBox( double* nNumb, BYTE* pSource, ULONG nSize )
{
    BOOL    bRetValue = FALSE;
    BYTE* pDest = ImplSearchEntry( pSource, (BYTE*)"%%BoundingBox:", nSize, 14 );
    if ( pDest )
    {
        nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
        pDest += 14;

        int nSizeLeft = nSize - ( pDest - pSource );
        if ( nSizeLeft > 100 )
            nSizeLeft = 100;    // only 100 bytes following the bounding box will be checked

        int i;
        for ( i = 0; ( i < 4 ) && nSizeLeft; i++ )
        {
            int     nDivision = 1;
            BOOL    bDivision = FALSE;
            BOOL    bNegative = FALSE;
            BOOL    bValid = TRUE;

            while ( ( --nSizeLeft ) && ( *pDest == ' ' ) || ( *pDest == 0x9 ) ) pDest++;
            BYTE nByte = *pDest;
            while ( nSizeLeft && ( nByte != ' ' ) && ( nByte != 0x9 ) && ( nByte != 0xd ) && ( nByte != 0xa ) )
            {
                switch ( nByte )
                {
                    case '.' :
                        if ( bDivision )
                            bValid = FALSE;
                        else
                            bDivision = TRUE;
                        break;
                    case '-' :
                        bNegative = TRUE;
                        break;
                    default :
                        if ( ( nByte < '0' ) || ( nByte > '9' ) )
                            nSizeLeft = 1;  // error parsing the bounding box values
                        else if ( bValid )
                        {
                            if ( bDivision )
                                nDivision*=10;
                            nNumb[i] *= 10;
                            nNumb[i] += nByte - '0';
                        }
                        break;
                }
                nSizeLeft--;
                nByte = *(++pDest);
            }
            if ( bNegative )
                nNumb[i] = -nNumb[i];
            if ( bDivision && ( nDivision != 1 ) )
                nNumb[i] /= nDivision;
        }
        if ( i == 4 )
            bRetValue = TRUE;
    }
    return bRetValue;
}

#if 0
static void ImplWriteDouble( BYTE** pBuf, double nNumber )
{
//  *pBuf += sprintf( (char*)*pBuf, "%f", nNumber );

    if ( nNumber < 0 )
    {
        *(*pBuf)++ = (BYTE)'-';
        nNumber = -nNumber;
    }
    ULONG nTemp = (ULONG)nNumber;
    const String aNumber1( nTemp );
    ULONG nLen = aNumber1.Len();

    for ( USHORT n = 0; n < nLen; n++ )
        *(*pBuf)++ = aNumber1[ n ];

    nTemp = (ULONG)( ( nNumber - nTemp ) * 100000 );
    if ( nTemp )
    {
        *(*pBuf)++ = (BYTE)'.';
        const String aNumber2( nTemp );

        ULONG nLen = aNumber2.Len();
        if ( nLen < 8 )
        {
            for ( n = 0; n < ( 5 - nLen ); n++ )
            {
                *(*pBuf)++ = (BYTE)'0';
            }
        }
        for ( USHORT n = 0; n < nLen; n++ )
        {
            *(*pBuf)++ = aNumber2[ n ];
        }
    }
    *(*pBuf)++ = ' ';
}
#endif

inline void ImplWriteString( BYTE** pBuf, const char* sString )
{
    strcpy( (char*)*pBuf, sString );
    *pBuf += strlen( sString );
}

BOOL Os2SalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
    if ( !mbPrinter )
        return FALSE;

    BOOL    bRet  = FALSE;
    LONG    nLong = 0;
    if ( !(DevQueryCaps( mhDC, CAPS_TECHNOLOGY, 1, &nLong ) &&
           (CAPS_TECH_POSTSCRIPT == nLong)) )
        return FALSE;

    BYTE*   pBuf = new BYTE[ POSTSCRIPT_BUFSIZE ];
    double  nBoundingBox[4];

    if ( pBuf && ImplGetBoundingBox( nBoundingBox, (BYTE*)pPtr, nSize ) )
    {
        LONG pOS2DXAry[4];        // hack -> print always 2 white space
        POINTL aPt;
        aPt.x = 0;
        aPt.y = 0;
        PCH pStr = (PCH) "  ";
        for( long i = 0; i < 4; i++ )
            pOS2DXAry[i] = i;
        Ft2CharStringPosAt( mhPS, &aPt, NULL, 0, 2, (PCH)pStr,(PLONG)&pOS2DXAry[0] );

        OStringBuffer aBuf( POSTSCRIPT_BUFSIZE );

                // reserve place for a USHORT
                aBuf.append( "aa" );

                // #107797# Write out EPS encapsulation header
                // ----------------------------------------------------------------------------------

                // directly taken from the PLRM 3.0, p. 726. Note:
                // this will definitely cause problems when
                // recursively creating and embedding PostScript files
                // in OOo, since we use statically-named variables
                // here (namely, b4_Inc_state_salWin, dict_count_salWin and
                // op_count_salWin). Currently, I have no idea on how to
                // work around that, except from scanning and
                // interpreting the EPS for unused identifiers.

                // append the real text
                aBuf.append( "\n\n/b4_Inc_state_salWin save def\n"
                             "/dict_count_salWin countdictstack def\n"
                             "/op_count_salWin count 1 sub def\n"
                             "userdict begin\n"
                             "/showpage {} def\n"
                             "0 setgray 0 setlinecap\n"
                             "1 setlinewidth 0 setlinejoin\n"
                             "10 setmiterlimit [] 0 setdash newpath\n"
                             "/languagelevel where\n"
                             "{\n"
                             "  pop languagelevel\n"
                             "  1 ne\n"
                             "  {\n"
                             "    false setstrokeadjust false setoverprint\n"
                             "  } if\n"
                             "} if\n\n" );

#if 0
                // #i10737# Apply clipping manually
                // ----------------------------------------------------------------------------------

                // Windows seems to ignore any clipping at the HDC,
                // when followed by a POSTSCRIPT_PASSTHROUGH

                // Check whether we've got a clipping, consisting of
                // exactly one rect (other cases should be, but aren't
                // handled currently)

                // TODO: Handle more than one rectangle here (take
                // care, the buffer can handle only POSTSCRIPT_BUFSIZE
                // characters!)
                if ( mhRegion != 0 &&
                     mpStdClipRgnData != NULL &&
                     mpClipRgnData == mpStdClipRgnData &&
                     mpClipRgnData->rdh.nCount == 1 )
                {
                    RECT* pRect = &(mpClipRgnData->rdh.rcBound);

                    aBuf.append( "\nnewpath\n" );
                    aBuf.append( pRect->left );
                    aBuf.append( " " );
                    aBuf.append( pRect->top );
                    aBuf.append( " moveto\n" );
                    aBuf.append( pRect->right );
                    aBuf.append( " " );
                    aBuf.append( pRect->top );
                    aBuf.append( " lineto\n" );
                    aBuf.append( pRect->right );
                    aBuf.append( " " );
                    aBuf.append( pRect->bottom );
                    aBuf.append( " lineto\n" );
                    aBuf.append( pRect->left );
                    aBuf.append( " " );
                    aBuf.append( pRect->bottom );
                    aBuf.append( " lineto\n"
                                 "closepath\n"
                                 "clip\n"
                                 "newpath\n" );
                }
#endif

                // #107797# Write out buffer
                // ----------------------------------------------------------------------------------
                *((USHORT*)aBuf.getStr()) = (USHORT)( aBuf.getLength() - 2 );
                //Escape ( mhDC, nEscape, aBuf.getLength(), (LPTSTR)aBuf.getStr(), 0 );
                DevEscape( mhDC, DEVESC_RAWDATA, aBuf.getLength(),
                        (PBYTE)aBuf.getStr(), 0, NULL );

        double dM11 = nWidth / ( nBoundingBox[2] - nBoundingBox[0] );
        double dM22 = - ( nHeight / (nBoundingBox[1] - nBoundingBox[3] ) );

                // reserve a USHORT again
                aBuf.setLength( 2 );
                aBuf.append( "\n\n[" );
                aBuf.append( dM11 );
                aBuf.append( " 0 0 " );
                aBuf.append( dM22 );
                aBuf.append( ' ' );
                aBuf.append( nX - ( dM11 * nBoundingBox[0] ) );
                aBuf.append( ' ' );
                aBuf.append( nY - ( dM22 * nBoundingBox[3] ) );
                aBuf.append( "] concat\n"
                             "%%BeginDocument:\n" );
                *((USHORT*)aBuf.getStr()) = (USHORT)( aBuf.getLength() - 2 );
                DevEscape( mhDC, DEVESC_RAWDATA, aBuf.getLength(),
                        (PBYTE)aBuf.getStr(), 0, NULL );
#if 0
        BYTE* pTemp = pBuf;
        ImplWriteString( &pTemp, "save\n[ " );
        ImplWriteDouble( &pTemp, dM11 );
        ImplWriteDouble( &pTemp, 0 );
        ImplWriteDouble( &pTemp, 0 );
        ImplWriteDouble( &pTemp, dM22 );
        ImplWriteDouble( &pTemp, nX - ( dM11 * nBoundingBox[0] ) );
        ImplWriteDouble( &pTemp, mnHeight - nY - ( dM22 * nBoundingBox[3] ) );
        ImplWriteString( &pTemp, "] concat /showpage {} def\n" );

        if ( DevEscape( mhDC, DEVESC_RAWDATA, pTemp - pBuf,
            (PBYTE)pBuf, 0, NULL ) == DEV_OK )
#endif //
        {
            UINT32 nToDo = nSize;
            UINT32 nDoNow;
            bRet = TRUE;
            while( nToDo && bRet )
            {
                nDoNow = 0x4000;
                if ( nToDo < nDoNow )
                    nDoNow = nToDo;

                if ( DevEscape( mhDC, DEVESC_RAWDATA, nDoNow, (PBYTE)pPtr + nSize - nToDo,
                   0, NULL ) == -1 )
                    bRet = FALSE;
                nToDo -= nDoNow;
            }

            if ( bRet )
            {
                strcpy ( (char*)pBuf, "\nrestore\n" );
                if ( DevEscape( mhDC, DEVESC_RAWDATA, 9, (PBYTE)pBuf,
                    0, NULL ) == DEV_OK ) bRet = TRUE;
            }

                // #107797# Write out EPS encapsulation footer
                // ----------------------------------------------------------------------------------
                // reserve a USHORT again
                aBuf.setLength( 2 );
                aBuf.append( "%%EndDocument\n"
                             "count op_count_salWin sub {pop} repeat\n"
                             "countdictstack dict_count_salWin sub {end} repeat\n"
                             "b4_Inc_state_salWin restore\n\n" );
                *((USHORT*)aBuf.getStr()) = (USHORT)( aBuf.getLength() - 2 );
                DevEscape( mhDC, DEVESC_RAWDATA, aBuf.getLength(),
                        (PBYTE)aBuf.getStr(), 0, NULL );
                bRet = TRUE;

        }
    }
    delete [] pBuf;
    return bRet;
}

/*
 * IsNativeControlSupported()
 *
 *  Returns TRUE if the platform supports native
 *  drawing of the control defined by nPart
 */
BOOL Os2SalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    return( FALSE );
}

// -----------------------------------------------------------------------

SystemGraphicsData Os2SalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
#if 0
    aRes.hDC = mhDC;
#endif
    return aRes;
}

// -----------------------------------------------------------------------
