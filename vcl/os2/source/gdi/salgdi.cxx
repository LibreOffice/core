/*************************************************************************
 *
 *  $RCSfile: salgdi.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
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

#include <string.h>
#include <tools/svpm.h>

#define _SV_SALGDI_CXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
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
}

// =======================================================================

void ImplSalInitGraphics( SalGraphicsData* pData )
{
    GpiCreateLogColorTable( pData->mhPS, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );
}

// -----------------------------------------------------------------------

void ImplSalDeInitGraphics( SalGraphicsData* pData )
{
}

// =======================================================================

SalGraphics::SalGraphics()
{
    maGraphicsData.mhPS             = 0;
    maGraphicsData.mhDC             = 0;
    maGraphicsData.mbLine               = FALSE;
    maGraphicsData.mbFill               = FALSE;
    maGraphicsData.mbXORMode            = FALSE;
    maGraphicsData.mbFontIsOutline      = FALSE;
    maGraphicsData.mbFontIsFixed        = FALSE;
    maGraphicsData.mnFontMetricCount    = 0;
    maGraphicsData.mpFontMetrics        = NULL;
    maGraphicsData.mpClipRectlAry       = NULL;
}

// -----------------------------------------------------------------------

SalGraphics::~SalGraphics()
{
    if ( maGraphicsData.mpFontMetrics )
        delete maGraphicsData.mpFontMetrics;
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

void SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    long nResolution;

    // convert resolution from pels per meter to pels per inch
    DevQueryCaps( maGraphicsData.mhDC, CAPS_HORIZONTAL_RESOLUTION, 1, &nResolution );
    rDPIX = (nResolution * 100) / 3937;

    // convert resolution from pels per meter to pels per inch
    DevQueryCaps( maGraphicsData.mhDC, CAPS_VERTICAL_RESOLUTION, 1, &nResolution );
    rDPIY = (nResolution * 100) / 3937;

    if ( rDPIY < 96 )
    {
        rDPIX = (rDPIX*96) / rDPIY;
        rDPIY = 96;
    }
}

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
    DevQueryCaps( maGraphicsData.mhDC, CAPS_HORIZONTAL_FONT_RES, 1, &rDPIX );
    DevQueryCaps( maGraphicsData.mhDC, CAPS_VERTICAL_FONT_RES, 1, &rDPIY );
}

// -----------------------------------------------------------------------

USHORT SalGraphics::GetBitCount()
{
    LONG nBitCount;
    DevQueryCaps( maGraphicsData.mhDC, CAPS_COLOR_BITCOUNT, 1, &nBitCount );
    return (USHORT)nBitCount;
}

// -----------------------------------------------------------------------

void SalGraphics::ResetClipRegion()
{
#ifdef SAL_PRINTER_CLIPPATH
    if ( maGraphicsData.mbPrinter )
        GpiSetClipPath( maGraphicsData.mhPS, 0, SCP_RESET );
    else
#endif
    {
        HRGN hOldRegion;

        GpiSetClipRegion( maGraphicsData.mhPS, NULL, &hOldRegion );
        if ( hOldRegion )
            GpiDestroyRegion( maGraphicsData.mhPS, hOldRegion );
    }
}

// -----------------------------------------------------------------------

void SalGraphics::BeginSetClipRegion( ULONG nCount )
{
    maGraphicsData.mpClipRectlAry     = new RECTL[ nCount ];
    maGraphicsData.mnClipElementCount = 0;
}

// -----------------------------------------------------------------------

BOOL SalGraphics::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    RECTL* pClipRect = &maGraphicsData.mpClipRectlAry[ maGraphicsData.mnClipElementCount ];
    pClipRect->xLeft   = nX;
    pClipRect->yTop    = maGraphicsData.mnHeight - nY;
    pClipRect->xRight  = nX + nWidth;
    pClipRect->yBottom = maGraphicsData.mnHeight - (nY + nHeight);
    maGraphicsData.mnClipElementCount++;

    return TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::EndSetClipRegion()
{
#ifdef SAL_PRINTER_CLIPPATH
    if ( maGraphicsData.mbPrinter )
    {
        GpiSetClipPath( maGraphicsData.mhPS, 0, SCP_RESET );
        GpiBeginPath( maGraphicsData.mhPS, 1L );

        for( int i = 0; i < maGraphicsData.mnClipElementCount; i++ )
        {
            POINTL aPt;
            RECTL* pClipRect = &maGraphicsData.mpClipRectlAry[ i ];

            aPt.x = pClipRect->xLeft;
            aPt.y = pClipRect->yTop-1;
            GpiMove( maGraphicsData.mhPS, &aPt );

            aPt.x = pClipRect->xRight-1;
            aPt.y = pClipRect->yBottom;

            GpiBox( maGraphicsData.mhPS, DRO_OUTLINE, &aPt, 0, 0 );
        }

        GpiEndPath( maGraphicsData.mhPS );
        GpiSetClipPath( maGraphicsData.mhPS, 1L, SCP_ALTERNATE | SCP_AND );
    }
    else
#endif
    {
        HRGN hClipRegion = GpiCreateRegion( maGraphicsData.mhPS,
                                            maGraphicsData.mnClipElementCount,
                                            maGraphicsData.mpClipRectlAry );
        HRGN hOldRegion;

        GpiSetClipRegion( maGraphicsData.mhPS, hClipRegion, &hOldRegion );
        if( hOldRegion )
            GpiDestroyRegion( maGraphicsData.mhPS, hOldRegion );
    }

    delete [] maGraphicsData.mpClipRectlAry;
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor()
{
    // don't draw line!
    maGraphicsData.mbLine = FALSE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor( SalColor nSalColor )
{
    LINEBUNDLE lb;

    // set color
    lb.lColor = RGBCOLOR( SALCOLOR_RED( nSalColor ),
                          SALCOLOR_GREEN( nSalColor ),
                          SALCOLOR_BLUE( nSalColor ) );

    GpiSetAttrs( maGraphicsData.mhPS,
                 PRIM_LINE,
                 LBB_COLOR,
                 0,
                 &lb );

    // draw line!
    maGraphicsData.mbLine = TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
    // don't fill area!
    maGraphicsData.mbFill = FALSE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
    AREABUNDLE ab;

    // set color
    ab.lColor = RGBCOLOR( SALCOLOR_RED( nSalColor ),
                          SALCOLOR_GREEN( nSalColor ),
                          SALCOLOR_BLUE( nSalColor ) );

    GpiSetAttrs( maGraphicsData.mhPS,
                 PRIM_AREA,
                 ABB_COLOR,
                 0,
                 &ab );

    // fill area!
    maGraphicsData.mbFill = TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetXORMode( BOOL bSet )
{
    maGraphicsData.mbXORMode = bSet;
    LONG nMixMode = bSet ? FM_XOR : FM_OVERPAINT;

    // set mix mode for lines
    LINEBUNDLE lb;
    lb.usMixMode = nMixMode;
    GpiSetAttrs( maGraphicsData.mhPS,
                 PRIM_LINE,
                 LBB_MIX_MODE,
                 0,
                 &lb );

    // set mix mode for areas
    AREABUNDLE ab;
    ab.usMixMode = nMixMode;
    GpiSetAttrs( maGraphicsData.mhPS,
                 PRIM_AREA,
                 ABB_MIX_MODE,
                 0,
                 &ab );

    // set mix mode for text
    CHARBUNDLE cb;
    cb.usMixMode = nMixMode;
    GpiSetAttrs( maGraphicsData.mhPS,
                 PRIM_CHAR,
                 CBB_MIX_MODE,
                 0,
                 &cb );
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    SetLineColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    SetFillColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY )
{
    POINTL aPt;

    aPt.x = nX;
    aPt.y = TY( nY );

    // set color
    GpiSetPel( maGraphicsData.mhPS, &aPt );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor )
{
    // save old color
    LINEBUNDLE oldLb;
    GpiQueryAttrs( maGraphicsData.mhPS,
                   PRIM_LINE,
                   LBB_COLOR,
                   &oldLb );

    // set new color
    LINEBUNDLE lb;
    lb.lColor = RGBCOLOR( SALCOLOR_RED( nSalColor ),
                          SALCOLOR_GREEN( nSalColor ),
                          SALCOLOR_BLUE( nSalColor ) );
    GpiSetAttrs( maGraphicsData.mhPS,
                 PRIM_LINE,
                 LBB_COLOR,
                 0,
                 &lb );

    // set color of pixel
    POINTL aPt;
    aPt.x = nX;
    aPt.y = TY( nY );
    GpiSetPel( maGraphicsData.mhPS, &aPt );

    // restore old color
    GpiSetAttrs( maGraphicsData.mhPS,
                 PRIM_LINE,
                 LBB_COLOR,
                 0,
                 &oldLb );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
    // OS2 zeichnet den Endpunkt mit
    POINTL aPt;
    aPt.x = nX1;
    aPt.y = TY( nY1 );
    GpiMove( maGraphicsData.mhPS, &aPt );
    aPt.x = nX2;
    aPt.y = TY( nY2 );
    GpiLine( maGraphicsData.mhPS, &aPt );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight )
{
    POINTL aPt;
    long lControl;

    if ( maGraphicsData.mbFill )
    {
        if ( maGraphicsData.mbLine )
            lControl = DRO_OUTLINEFILL;
        else
            lControl = DRO_FILL;
    }
    else
    {
        if ( maGraphicsData.mbLine )
            lControl = DRO_OUTLINE;
        else
            return;
    }

    aPt.x = nX;
    aPt.y = TY( nY );
    GpiMove( maGraphicsData.mhPS, &aPt );
    aPt.x = nX + nWidth - 1;
    aPt.y = TY( nY + nHeight - 1 );
    GpiBox( maGraphicsData.mhPS, lControl, &aPt, 0, 0 );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry )
{
    // convert all points to sys orientation
    POINTL*             pOS2PtAry = new POINTL[ nPoints ];
    POINTL*             pTempOS2PtAry = pOS2PtAry;
    const SalPoint*     pTempPtAry = pPtAry;
    ULONG               nTempPoints = nPoints;
    long                nHeight = maGraphicsData.mnHeight - 1;

    while( nTempPoints-- )
    {
        (*pTempOS2PtAry).x = (*pTempPtAry).mnX;
        (*pTempOS2PtAry).y = nHeight - (*pTempPtAry).mnY;
        pTempOS2PtAry++;
        pTempPtAry++;
    }

    GpiMove( maGraphicsData.mhPS, pOS2PtAry );
    GpiPolyLine( maGraphicsData.mhPS, nPoints, pOS2PtAry );
    delete [] pOS2PtAry;
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    POLYGON aPolygon;

    // create polygon
    aPolygon.aPointl = new POINTL[ nPoints ];
    aPolygon.ulPoints = nPoints;

    // convert all points to sys orientation
    POINTL*             pTempOS2PtAry = aPolygon.aPointl;
    const SalPoint*     pTempPtAry = pPtAry;
    ULONG               nTempPoints = nPoints;
    long                nHeight = maGraphicsData.mnHeight - 1;

    while( nTempPoints-- )
    {
        (*pTempOS2PtAry).x = (*pTempPtAry).mnX;
        (*pTempOS2PtAry).y = nHeight - (*pTempPtAry).mnY;
        pTempOS2PtAry++;
        pTempPtAry++;
    }

    // Innenleben zeichnen
    if ( maGraphicsData.mbFill )
    {
#ifdef SAL_PRINTER_POLYPATH
        if ( maGraphicsData.mbPrinter )
        {
            GpiBeginPath( maGraphicsData.mhPS, 1 );
            GpiMove( maGraphicsData.mhPS, aPolygon.aPointl );
            GpiPolyLine( maGraphicsData.mhPS, aPolygon.ulPoints, aPolygon.aPointl );
            GpiEndPath( maGraphicsData.mhPS );
            GpiFillPath( maGraphicsData.mhPS, 1, 0 );

            if ( maGraphicsData.mbLine )
            {
                GpiMove( maGraphicsData.mhPS, aPolygon.aPointl );
                GpiPolyLine( maGraphicsData.mhPS, aPolygon.ulPoints, aPolygon.aPointl );
            }
        }
        else
#endif
        {
            ULONG nOptions = POLYGON_ALTERNATE;

            if ( maGraphicsData.mbLine )
                nOptions |= POLYGON_BOUNDARY;
            else
                nOptions |= POLYGON_NOBOUNDARY;

            GpiMove( maGraphicsData.mhPS, aPolygon.aPointl );
            GpiPolygons( maGraphicsData.mhPS, 1, &aPolygon, nOptions, POLYGON_EXCL );
        }
    }
    else
    {
        if ( maGraphicsData.mbLine )
        {
            GpiMove( maGraphicsData.mhPS, aPolygon.aPointl );
            GpiPolyLine( maGraphicsData.mhPS, nPoints, aPolygon.aPointl );
        }
    }

    delete [] aPolygon.aPointl;
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
                                   PCONSTSALPOINT* pPtAry )
{
    ULONG       i;
    long        nHeight = maGraphicsData.mnHeight - 1;
    POLYGON*    aPolygonAry = new POLYGON[ nPoly ];

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
    if ( maGraphicsData.mbFill )
    {
#ifdef SAL_PRINTER_POLYPATH
        if ( maGraphicsData.mbPrinter )
        {
            GpiBeginPath( maGraphicsData.mhPS, 1 );
            for ( i = 0; i < nPoly; i++ )
            {
                GpiMove( maGraphicsData.mhPS, aPolygonAry[i].aPointl );
                GpiPolyLine( maGraphicsData.mhPS, aPolygonAry[i].ulPoints, aPolygonAry[i].aPointl );
            }
            GpiEndPath( maGraphicsData.mhPS );
            GpiFillPath( maGraphicsData.mhPS, 1, 0 );
        }
        else
#endif
        {
            ULONG nOptions = POLYGON_ALTERNATE;

            if ( maGraphicsData.mbLine )
                nOptions |= POLYGON_BOUNDARY;
            else
                nOptions |= POLYGON_NOBOUNDARY;

            GpiMove( maGraphicsData.mhPS, aPolygonAry[ 0 ].aPointl );
            GpiPolygons( maGraphicsData.mhPS, nPoly, aPolygonAry, nOptions, POLYGON_EXCL );
        }
    }
    else
    {
        if ( maGraphicsData.mbLine )
        {
            for( i = 0; i < nPoly; i++ )
            {
                GpiMove( maGraphicsData.mhPS, aPolygonAry[ i ].aPointl );
                GpiPolyLine( maGraphicsData.mhPS, aPolygonAry[ i ].ulPoints, aPolygonAry[ i ].aPointl );
            }
        }
    }

    // cleanup
    for( i = 0; i < nPoly; i++ )
        delete [] aPolygonAry[ i ].aPointl;
    delete [] aPolygonAry;
}

// =======================================================================

// MAXIMUM BUFSIZE EQ 0xFFFF
#define POSTSCRIPT_BUFSIZE          0x1024
// we only try to get the BoundingBox in the first 4096 bytes
#define POSTSCRIPT_BOUNDINGSEARCH   0x1000

static BYTE* ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize )
{
    while ( nComp-- >= nSize )
    {
        for ( ULONG i = 0; i < nSize; i++ )
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
    ULONG   nBytesRead;

    if ( nSize < 256 )      // we assume that the file is greater than 256 bytes
        return FALSE;

    if ( nSize < POSTSCRIPT_BOUNDINGSEARCH )
        nBytesRead = nSize;
    else
        nBytesRead = POSTSCRIPT_BOUNDINGSEARCH;

    BYTE* pDest = ImplSearchEntry( pSource, (BYTE*)"%%BoundingBox:", nBytesRead, 14 );
    if ( pDest )
    {
        int     nSecurityCount = 100;   // only 100 bytes following the bounding box will be checked
        nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
        pDest += 14;
        for ( int i = 0; ( i < 4 ) && nSecurityCount; i++ )
        {
            int     nDivision = 1;
            BOOL    bDivision = FALSE;
            BOOL    bNegative = FALSE;
            BOOL    bValid = TRUE;

            while ( ( --nSecurityCount ) && ( *pDest == ' ' ) || ( *pDest == 0x9 ) ) pDest++;
            BYTE nByte = *pDest;
            while ( nSecurityCount && ( nByte != ' ' ) && ( nByte != 0x9 ) && ( nByte != 0xd ) && ( nByte != 0xa ) )
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
                            nSecurityCount = 1;     // error parsing the bounding box values
                        else if ( bValid )
                        {
                            if ( bDivision )
                                nDivision*=10;
                            nNumb[i] *= 10;
                            nNumb[i] += nByte - '0';
                        }
                        break;
                }
                nSecurityCount--;
                nByte = *(++pDest);
            }
            if ( bNegative )
                nNumb[i] = -nNumb[i];
            if ( bDivision && ( nDivision != 1 ) )
                nNumb[i] /= nDivision;
        }
        if ( nSecurityCount)
            bRetValue = TRUE;
    }
    return bRetValue;
}

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

inline void ImplWriteString( BYTE** pBuf, const char* sString )
{
    strcpy( (char*)*pBuf, sString );
    *pBuf += strlen( sString );
}

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
    if ( !maGraphicsData.mbPrinter )
        return FALSE;

    BOOL    bRet  = FALSE;
    LONG    nLong = 0;
    if ( !(DevQueryCaps( maGraphicsData.mhDC, CAPS_TECHNOLOGY, 1, &nLong ) &&
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
        PCH pStr = "  ";
        for( long i = 0; i < 4; i++ )
            pOS2DXAry[i] = i;
        GpiCharStringPosAt( maGraphicsData.mhPS, &aPt, NULL, 0, 2, (PCH)pStr,(PLONG)&pOS2DXAry[0] );

        double dM11 = nWidth / ( nBoundingBox[2] - nBoundingBox[0] );
        double dM22 = - ( nHeight / (nBoundingBox[1] - nBoundingBox[3] ) );

        BYTE* pTemp = pBuf;
        ImplWriteString( &pTemp, "save\n[ " );
        ImplWriteDouble( &pTemp, dM11 );
        ImplWriteDouble( &pTemp, 0 );
        ImplWriteDouble( &pTemp, 0 );
        ImplWriteDouble( &pTemp, dM22 );
        ImplWriteDouble( &pTemp, nX - ( dM11 * nBoundingBox[0] ) );
        ImplWriteDouble( &pTemp, maGraphicsData.mnHeight - nY - ( dM22 * nBoundingBox[3] ) );
        ImplWriteString( &pTemp, "] concat /showpage {} def\n" );

        if ( DevEscape( maGraphicsData.mhDC, DEVESC_RAWDATA, pTemp - pBuf,
            (PBYTE)pBuf, 0, NULL ) == DEV_OK )
        {
            UINT32 nToDo = nSize;
            UINT32 nDoNow;
            bRet = TRUE;
            while( nToDo && bRet )
            {
                nDoNow = 0x4000;
                if ( nToDo < nDoNow )
                    nDoNow = nToDo;

                if ( DevEscape( maGraphicsData.mhDC, DEVESC_RAWDATA, nDoNow, (PBYTE)pPtr + nSize - nToDo,
                   0, NULL ) == -1 )
                    bRet = FALSE;
                nToDo -= nDoNow;
            }

            if ( bRet )
            {
                strcpy ( (char*)pBuf, "\nrestore\n" );
                if ( DevEscape( maGraphicsData.mhDC, DEVESC_RAWDATA, 9, (PBYTE)pBuf,
                    0, NULL ) == DEV_OK ) bRet = TRUE;
            }
        }
    }
    delete pBuf;
    return bRet;
}

