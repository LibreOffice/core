/*************************************************************************
 *
 *  $RCSfile: salgdi.cxx,v $
 *
 *  $Revision: 1.32 $
 *  last change: $Author: bmahbod $ $Date: 2000-12-21 19:37:41 $
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

#define _SV_SALGDI_CXX

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_VCLGRAPHICS_H
#include <VCLGraphics.h>
#endif

// =======================================================================

static inline unsigned long AbsoluteValue ( const long  nValue ){ unsigned long   nAbsValue = 0;        if ( nValue < 0 )  {      nAbsValue = -nValue;   } // if    else   {      nAbsValue = nValue;    } // else             return nAbsValue;} // AbsoluteValue

// =======================================================================

// =======================================================================

static void SetWhiteBackColor()
{
    RGBColor aWhiteBackColor;

    // White color

    aWhiteBackColor.red   = 0xffff;
    aWhiteBackColor.green = 0xffff;
    aWhiteBackColor.blue  = 0xffff;

    // Set background color to white

    RGBBackColor( &aWhiteBackColor );
} // SetWhiteBackColor

// -----------------------------------------------------------------------

static void SetBlackForeColor()
{
    RGBColor aBlackForeColor;

    // Black color

    aBlackForeColor.red   = 0x0000;
    aBlackForeColor.green = 0x0000;
    aBlackForeColor.blue  = 0x0000;

    // Set foreground color to black

    RGBForeColor( &aBlackForeColor );
} // SetBlackForeColor

// =======================================================================

// =======================================================================

static void CheckRectBounds ( Rect        *rSrcRect,
                              Rect        *rDstRect,
                              const Rect  *rPortBoundsRect
                            )

{
    if ( rSrcRect->top < rPortBoundsRect->top )
    {
        rDstRect->top += (rPortBoundsRect->top - rSrcRect->top);
        rSrcRect->top  = rPortBoundsRect->top;
    } // if

    if ( rSrcRect->left < rPortBoundsRect->left )
    {
        rDstRect->left += (rPortBoundsRect->left - rSrcRect->left);
        rSrcRect->left  = rPortBoundsRect->left;
    } // if

    if ( rSrcRect->bottom > rPortBoundsRect->bottom )
    {

        rDstRect->bottom += (rPortBoundsRect->bottom - rSrcRect->bottom);
        rSrcRect->bottom  = rPortBoundsRect->bottom;
    } // if

    if ( rSrcRect->right > rPortBoundsRect->right )
    {
        rDstRect->right += (rPortBoundsRect->right - rSrcRect->right);
        rSrcRect->right  = rPortBoundsRect->right;
    } // if
} // CheckRectBounds

// -----------------------------------------------------------------------

static inline void GetQDRect ( const long   nX,
                               const long   nY,
                               const long   nWidth,
                               const long   nHeight,
                               Rect        *rRect
                             )
{
    SetRect( rRect,  nX, nY, nX + nWidth, nY + nHeight );
} // GetQDRect

// -----------------------------------------------------------------------

static inline void SalTwoRect2SQDDstRect ( const SalTwoRect  *pPosAry,
                                           Rect              *rDstRect
                                         )
{
    SetRect( rDstRect,
             pPosAry->mnDestX,
             pPosAry->mnDestY,
             pPosAry->mnDestX + pPosAry->mnDestWidth,
             pPosAry->mnDestY + pPosAry->mnDestHeight
           );
} // GetSalSourceRect

// -----------------------------------------------------------------------

static inline void SalTwoRect2SQDSrcRect ( const SalTwoRect  *pPosAry,
                                           Rect              *rSrcRect
                                         )
{
    SetRect( rSrcRect,
             pPosAry->mnSrcX,
             pPosAry->mnSrcY,
             pPosAry->mnSrcX + pPosAry->mnSrcWidth,
             pPosAry->mnSrcY + pPosAry->mnSrcHeight
           );
} // SalTwoRect2SQDSrcRect

// -----------------------------------------------------------------------

static inline short SelectCopyMode ( const SalGraphicsDataPtr pSalGraphicsData )
{
    short nCopyMode = 0;

    if ( pSalGraphicsData->mnPenMode == patCopy )
    {
        nCopyMode = srcCopy;
    } // if
    else
    {
        nCopyMode = srcXor;
    } // else

    return nCopyMode;
} // SelectCopyMode

// =======================================================================

// =======================================================================
static unsigned long  Compress32BitRGBColor ( const RGBColor *pRGBColor ){   unsigned long nCTableIndex = 0;

    nCTableIndex  = ( pRGBColor->red   >> kOneByte ) << kTwoBytes; nCTableIndex |= ( pRGBColor->green >> kOneByte ) << kOneByte;  nCTableIndex |=   pRGBColor->blue  >> kOneByte;       return nCTableIndex;} // Compress32BitRGBColor    // -----------------------------------------------------------------------

static unsigned long  Compress16BitRGBColor ( const RGBColor *pRGBColor ){    unsigned long nCTableIndex = 0;           nCTableIndex  = ( pRGBColor->red   >> kElevenBits ) << kTenBits;   nCTableIndex |= ( pRGBColor->green >> kElevenBits ) << kFiveBits;  nCTableIndex |=   pRGBColor->blue  >> kElevenBits;        return nCTableIndex;} // Compress16BitRGBColor    // -----------------------------------------------------------------------

static void CompressRGBColor ( const PixMapPtr   pPixMap,
                               const RGBColor   *pRGBColor,                               SalColor         *rSalColor
                             ){
    if ( pPixMap->pixelSize == k32BitScreenDepth ) {      *rSalColor = Compress32BitRGBColor( pRGBColor );   } // if    else   {      *rSalColor = Compress16BitRGBColor( pRGBColor );   } // else} // CompressRGBColor    // -----------------------------------------------------------------------

static unsigned long  RGBDistance ( const RGBColor  *pRGBColor1,                                    const RGBColor  *pRGBColor2
                                  ){  unsigned long   nRGBDist    = 0;   long            nDeltaRed   = 0;   long            nDeltaGreen = 0;   long            nDeltaBlue  = 0;          nDeltaRed   = (long)pRGBColor2->red   - (long)pRGBColor1->red; nDeltaGreen = (long)pRGBColor2->green - (long)pRGBColor1->green;   nDeltaBlue  = (long)pRGBColor2->blue  - (long)pRGBColor1->blue;           nRGBDist =   AbsoluteValue(nDeltaRed)         + AbsoluteValue(nDeltaGreen)           + AbsoluteValue(nDeltaBlue);           return nRGBDist;} // RGBDistance
// -----------------------------------------------------------------------

static BOOL QDColorsMatch ( const RGBColor  *pRGBColor1,                            const RGBColor  *pRGBColor2
                          ){  if (    ( pRGBColor2->red   == pRGBColor1->red   )          && ( pRGBColor2->green == pRGBColor1->green )      && ( pRGBColor2->blue  == pRGBColor1->blue  )
           )
    {
        return TRUE;
    } // if
    else
    {
        return FALSE;
    } // else} // QDColorsMatch

// -----------------------------------------------------------------------

static void GetBestSalColor ( const CTabPtr    pCTable,
                              const RGBColor  *pRGBColor,                              unsigned long   *pBestSalColor
                            ){    unsigned short   nCTableSize     = 0;  unsigned long    nCTableIndex    = 0;  unsigned long    nRGBNewDistance = 0;  unsigned long    nRGBMinDistance = UID_MAX;    RGBColor        *pRGBNextColor   = NULL;   BOOL             bRGBColorsMatch = FALSE; *pBestSalColor = 0;           nCTableSize = pCTable->ctSize;        while ( ( nCTableIndex < nCTableSize ) && ( bRGBColorsMatch == FALSE ) )   {      pRGBNextColor = &pCTable->ctTable[nCTableIndex].rgb;
        bRGBColorsMatch = QDColorsMatch( pRGBColor, pRGBNextColor );
       if ( bRGBColorsMatch == TRUE )     {          *pBestSalColor= nCTableIndex;      } // if        else       {          nRGBNewDistance = RGBDistance( pRGBColor, pRGBNextColor );                        if ( nRGBNewDistance < nRGBMinDistance )           {               nRGBMinDistance = nRGBNewDistance;                *pBestSalColor   = nCTableIndex;           } // if                               nCTableIndex++;        } //else   } // while} // GetBestSalColor    // -----------------------------------------------------------------------

static void GetCTableIndex ( const PixMapPtr   pPixMap,
                             const RGBColor   *pRGBColor,                             SalColor         *rSalColor
                           ){ CTabPtr pCTable = NULL;   pCTable = *(*pPixMap).pmTable;
    if ( pCTable != NULL )
    {          GetBestSalColor( pCTable, pRGBColor, rSalColor );                                                                 if ( *rSalColor > pCTable->ctSize )        {          *rSalColor = 255;      } // if
    } // if} // GetCTableIndex    // -----------------------------------------------------------------------
//
// Convert RGB color to Sal color.
//
// -----------------------------------------------------------------------

static SalColor RGBColor2SALColor ( const RGBColor *pRGBColor ){  GDPtr      pGDevice  = NULL;   SalColor   nSalColor = 0;                 pGDevice = *GetGDevice ( );
    if ( pGDevice != NULL )
    {
        PixMapPtr  pPixMap = NULL;

        pPixMap = *(*pGDevice).gdPMap;

        if ( pPixMap != NULL )
        {          if ( pGDevice->gdType == directType )          {              CompressRGBColor( pPixMap, pRGBColor, &nSalColor );            } // if            else           {              GetCTableIndex( pPixMap, pRGBColor, &nSalColor );          } // else
        } // if
    } // if                   return  nSalColor;} // RGBColor2SALColor// =======================================================================
// =======================================================================
static unsigned short SalColorTo32BitDeviceColor ( SalColor  *rSalColor ){   unsigned short  nDirectColor = 0;  unsigned short  nUpperByte   = 0;  unsigned short  nLowerByte   = 0;

     nLowerByte     = *rSalColor   & kByteMask;     nUpperByte     =  nLowerByte << kOneByte;  nDirectColor   =  nUpperByte  | nLowerByte;   *rSalColor    >>=  kOneByte;          return  nDirectColor;} // SalColorTo32BitDeviceColor

// -----------------------------------------------------------------------

static void SalColor2DirectColor ( const SalColor   nSalColor,                                   RGBColor        *rRGBColor
                                 ){
    SalColor  nIndexedColor = nSalColor;
   rRGBColor->blue  = SalColorTo32BitDeviceColor( &nIndexedColor );   rRGBColor->green = SalColorTo32BitDeviceColor( &nIndexedColor );   rRGBColor->red   = SalColorTo32BitDeviceColor( &nIndexedColor );
} // SalColor2DirectColor      
// -----------------------------------------------------------------------

static void SalColor2EightBitColor ( const GDPtr      pGDevice,
                                     const SalColor   nSalColor,                                     RGBColor        *rRGBColor
                                   ){ CTabPtr    pCTable = NULL; PixMapPtr  pPixMap = NULL;        pPixMap = *(*pGDevice).gdPMap;

    if ( pPixMap != NULL )
    {      pCTable = *(*pPixMap).pmTable;

        if ( pCTable != NULL )
        {          if ( nSalColor <= pCTable->ctSize )            {              RGBColor  aRGBColor;                      aRGBColor        = pCTable->ctTable[nSalColor].rgb;                rRGBColor->red   = aRGBColor.red;              rRGBColor->green = aRGBColor.green;                rRGBColor->blue  = aRGBColor.blue;         } // if
        } // if
    } // if} // SalColor2EightBitColor
// -----------------------------------------------------------------------
//
// Here we will convert SAL color to either 8-bit or 32-bit color.
// For 16-bit color we shall let Mac OS compute the nearest color
// from that of 32-bit color using the Euclidean 2-norm in RGB color
// space.
//
// -----------------------------------------------------------------------

static RGBColor SALColor2RGBColor ( const SalColor nSalColor )
{
    GDPtr     pGDevice = NULL; RGBColor  aRGBColor;
           memset( &aRGBColor, 0, sizeof(RGBColor) );
   pGDevice = *GetGDevice();
    if ( pGDevice != NULL )
    {          if ( pGDevice->gdType == directType )      {          SalColor2DirectColor( nSalColor, &aRGBColor );     } // if        else       {          SalColor2EightBitColor( pGDevice, nSalColor, &aRGBColor );     } // else
    } // if

    return aRGBColor;
} // SALColor2RGBColor

// =======================================================================

// =======================================================================

static OSStatus GetGDeviceBitDepth ( unsigned short  *rGDeviceBitDepth )
{
    GDPtr     pGDevice  = NULL;    OSStatus  nQDStatus = noErr;                                  pGDevice = *GetGDevice ( );
    nQDStatus = QDErr();
    if ( ( pGDevice != NULL ) && ( nQDStatus == noErr ) )
    {
        PixMapPtr  pPixMap = NULL;

        pPixMap = *pGDevice->gdPMap;

        if ( pPixMap != NULL )
        {
            // From the PixMap data get the current bits-per-pixel
            // as associated with the current GDevice

            *rGDeviceBitDepth = (unsigned short)pPixMap->pixelSize;
        } // if
    } // if

    return nQDStatus;
} // GetGDeviceBitDepth

// -----------------------------------------------------------------------

static OSStatus GetGDeviceResolution ( long  *rGDeviceHRes,
                                       long  *rGDeviceVRes
                                     )
{
    GDPtr     pGDevice  = NULL;
    OSStatus  nQDStatus = noErr;                  pGDevice = *GetGDevice ( );

    nQDStatus = QDErr();
    if ( ( pGDevice != NULL ) && ( nQDStatus == noErr ) )
    {
        long  nGDeviceTop    = pGDevice->gdRect.top;
        long  nGDeviceLeft   = pGDevice->gdRect.left;
        long  nGDeviceBottom = pGDevice->gdRect.bottom;
        long  nGDeviceRight  = pGDevice->gdRect.right;

        // From the current GDevice get its horizontal resolution

        *rGDeviceHRes = AbsoluteValue( nGDeviceRight - nGDeviceLeft );

        // From the current GDevice get its vertical resolution

        *rGDeviceVRes = AbsoluteValue( nGDeviceBottom - nGDeviceTop );
    } // if

    return nQDStatus;
} // GetGDeviceResolution

// =======================================================================

// =======================================================================

static OSStatus BeginClip ( SalGraphicsDataPtr rSalGraphicsData )
{
    if (    ( rSalGraphicsData->mbClipRegionChanged == TRUE )
         && ( rSalGraphicsData->mhClipRgn != NULL )
       )
    {
        Rect aClipRect;

        // Get the port bounds from our current region handle

        GetRegionBounds( rSalGraphicsData->mhClipRgn, &aClipRect );

        // Clip to a rectangle that we got from our current region

        ClipRect( &aClipRect );

        // Was there an error after clipping to a rectangle?

        rSalGraphicsData->mnMacOSStatus == QDErr();
    } // if

    return rSalGraphicsData->mnMacOSStatus;
} // BeginClip

// -----------------------------------------------------------------------

static OSStatus EndClip ( SalGraphicsDataPtr rSalGraphicsData )
{
    if ( rSalGraphicsData->mhClipRgn != NULL )
    {
        // Set the clip region

        SetClip( rSalGraphicsData->mhClipRgn );

        // Was there an error after setting the clipping region?

        rSalGraphicsData->mnMacOSStatus = QDErr();

        // Set the new status flag for our port

        rSalGraphicsData->mbClipRegionChanged = FALSE;
    } // if

    return rSalGraphicsData->mnMacOSStatus;
} // EndClip

// =======================================================================

// =======================================================================

static OSStatus OpenQDPort ( SalGraphicsDataPtr rSalGraphicsData )
{
    // Previous to entering this function, was there a QD error?

    if ( rSalGraphicsData->mnMacOSStatus == noErr )
    {
        // Get the graph port and lock focus on it

        rSalGraphicsData->mpCGrafPort = VCLGraphics_LockFocusCGrafPort( rSalGraphicsData->mhDC );

        if ( rSalGraphicsData->mpCGrafPort != NULL )
        {
            // Set to the current graph port

            MacSetPort( rSalGraphicsData->mpCGrafPort );

            // Was there a QD error when we set the port?

            rSalGraphicsData->mnMacOSStatus = QDErr();
        } // if
    } // if

    return rSalGraphicsData->mnMacOSStatus;
} // OpenQDPort

// -----------------------------------------------------------------------

static OSStatus CloseQDPort ( SalGraphicsDataPtr rSalGraphicsData )
{
    // Previous to entering this function, was there a QD error?

    if ( rSalGraphicsData->mnMacOSStatus == noErr )
    {
        // Flush the QuickDraw buffer

        QDFlushPortBuffer( rSalGraphicsData->mpCGrafPort, NULL );

        // Unlock focus on the current NSView

        VCLGraphics_UnLockFocusCGrafPort( rSalGraphicsData->mhDC );

        // When we get here then the QD port must have changed(?)

        PortChanged( rSalGraphicsData->mpCGrafPort );

        // Was there a QD error?

        rSalGraphicsData->mnMacOSStatus = QDErr();
    } // if

    return rSalGraphicsData->mnMacOSStatus;
} // CloseQDPort

// =======================================================================

// =======================================================================

static void InitBrush ( SalGraphicsDataPtr rSalGraphicsData )
{
    RGBColor aBlackColor;

    aBlackColor.red   = 0x0000;
    aBlackColor.green = 0x0000;
    aBlackColor.blue  = 0x0000;

    rSalGraphicsData->mbTransparentBrush = FALSE;
    rSalGraphicsData->maBrushColor       = aBlackColor;
} // InitBrush

// -----------------------------------------------------------------------

static void InitFont ( SalGraphicsDataPtr rSalGraphicsData )
{
    RGBColor aBlackColor;

    aBlackColor.red   = 0x0000;
    aBlackColor.green = 0x0000;
    aBlackColor.blue  = 0x0000;

    rSalGraphicsData->maFontColor = aBlackColor;
    rSalGraphicsData->mnFontID    = kFontIDGeneva;
    rSalGraphicsData->mnFontSize  = 10;
    rSalGraphicsData->mnFontStyle = normal;
} // InitFont

// -----------------------------------------------------------------------

static void InitPen ( SalGraphicsDataPtr rSalGraphicsData )
{
    RGBColor aBlackColor;

    aBlackColor.red   = 0x0000;
    aBlackColor.green = 0x0000;
    aBlackColor.blue  = 0x0000;

    rSalGraphicsData->maPenColor       = aBlackColor;
    rSalGraphicsData->mnPenMode        = patCopy;
    rSalGraphicsData->mbTransparentPen = FALSE;
} // InitPen

// -----------------------------------------------------------------------

static void InitQD ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->mpCGrafPort       = NULL;
    rSalGraphicsData->mpOffscreenGWorld = NULL;
    rSalGraphicsData->mhGDevice         = NULL;
} // InitQD

// -----------------------------------------------------------------------

static void InitRegions ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->mhClipRgn = NULL;
    rSalGraphicsData->mhGrowRgn = NewRgn();
} // InitRegions

// -----------------------------------------------------------------------

static void InitStatusFlags ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->mnMacOSStatus = noErr;
} // InitStatusFlags

// =======================================================================

// =======================================================================

static RgnHandle GetPolygonRgn ( const unsigned long   nPolyCount,
                                 const unsigned long  *pPoints,
                                 PCONSTSALPOINT       *ppPtAry,
                                 OSStatus             *rQDStatus
                               )
{
    RgnHandle hSrcRgnA = NULL;

    hSrcRgnA = NewRgn();

    if ( hSrcRgnA != NULL )
    {
        RgnHandle hSrcRgnB = NULL;

        hSrcRgnB = NewRgn();

        if ( hSrcRgnB != NULL )
        {
            unsigned short  nPolyEdgeIndex = 0;
            unsigned short  nPolyIndex     = 0;
            unsigned short  nPolyEdges     = 0;

            for ( nPolyIndex = 0; nPolyIndex < nPolyCount; nPolyIndex ++ )
            {
                const SalPoint *pPtAry = ppPtAry[nPolyIndex];

                nPolyEdges = pPoints[nPolyIndex];

                // Begin region construction

                OpenRgn();

                    // Begin polygon construction

                    MoveTo ( pPtAry[0].mnX,  pPtAry[0].mnY );

                    for ( nPolyEdgeIndex = 1;
                          nPolyEdgeIndex < nPolyEdges;
                          nPolyEdgeIndex++
                        )
                    {
                        MacLineTo( pPtAry[nPolyEdgeIndex].mnX,
                                   pPtAry[nPolyEdgeIndex].mnY
                                 );
                    } // for

                    MacLineTo( pPtAry[0].mnX,  pPtAry[0].mnY );

                    // End polygon construction

                CloseRgn( hSrcRgnB );

                // End region construction

                *rQDStatus = QDErr();

                if ( *rQDStatus == noErr )
                {
                    if ( nPolyIndex == 0 )
                    {
                        MacCopyRgn( hSrcRgnB, hSrcRgnA );
                    } // if
                    else
                    {
                        MacXorRgn( hSrcRgnA, hSrcRgnB, hSrcRgnA );
                    } // else
                } // if
            } // for

            DisposeRgn( hSrcRgnB );

            *rQDStatus = QDErr();

            if ( *rQDStatus != noErr )
            {
                DisposeRgn( hSrcRgnA );

                hSrcRgnA = NULL;
            }

            hSrcRgnB = NULL;
        } // if
    } // if

    return hSrcRgnA;
} // GetPolygonRgn

// =======================================================================

// =======================================================================

SalGraphics::SalGraphics()
{
    // QuickDraw graph port, offscreen graphic world, and graphic device handle

    InitQD( &maGraphicsData );

    // Regions within a current port

    InitRegions( &maGraphicsData );

    // Font attributes

    InitFont(  &maGraphicsData );

    // Pen attributes and status

    InitPen( &maGraphicsData );

    // Brush attributes and status

    InitBrush(  &maGraphicsData );

    // Miscellaneous status flags

    InitStatusFlags(  &maGraphicsData );

    // Set background and foreground colors on this graph port

    SetWhiteBackColor();
    SetBlackForeColor();
} // SalGraphics Class Constructor

// -----------------------------------------------------------------------

SalGraphics::~SalGraphics()
{
    if ( maGraphicsData.mpCGrafPort != NULL )
    {
        DisposePort( maGraphicsData.mpCGrafPort );
    } // if

    if ( maGraphicsData.mhClipRgn != NULL )
    {
        DisposeRgn( maGraphicsData.mhClipRgn );
    } // if

    if ( maGraphicsData.mhGrowRgn != NULL )
    {
        DisposeRgn( maGraphicsData.mhGrowRgn );
    } // if
} // SalGraphics Class Destructor

// =======================================================================

// =======================================================================

void SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    long nHRes = 0;
    long nVRes = 0;

    maGraphicsData.mnMacOSStatus = GetGDeviceResolution( &nHRes, &nVRes );

    if ( maGraphicsData.mnMacOSStatus == noErr )
    {
        rDPIX = nHRes;
        rDPIY = nVRes;
    } // if
} // SalGraphics::GetResolution

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
    long nHRes = 0;
    long nVRes = 0;

    maGraphicsData.mnMacOSStatus = GetGDeviceResolution( &nHRes, &nVRes );

    if ( maGraphicsData.mnMacOSStatus == noErr )
    {
        rDPIX = nHRes;
        rDPIY = nVRes;
    } // if
} // SalGraphics::GetScreenFontResolution

// -----------------------------------------------------------------------

USHORT SalGraphics::GetBitCount()
{
    unsigned short nBitDepth = 0;

    maGraphicsData.mnMacOSStatus = GetGDeviceBitDepth ( &nBitDepth );

    return nBitDepth;
} // SalGraphics::GetBitCount

// -----------------------------------------------------------------------

void SalGraphics::ResetClipRegion()
{
    if ( !maGraphicsData.mbWindow )
    {
        if ( maGraphicsData.mhClipRgn != NULL )
        {
            DisposeRgn( maGraphicsData.mhClipRgn );

            maGraphicsData.mhClipRgn = NULL;
        } // if
    } // if
    else
    {
        Rect   aClipRect;
        short  nLeft   = 0;
        short  nTop    = 0;
        short  nRight  = 0;
        short  nBottom = 0;

        if ( maGraphicsData.mhClipRgn == NULL )
        {
            maGraphicsData.mhClipRgn = NewRgn();
        } // if

        GetPortBounds( maGraphicsData.mpCGrafPort, &aClipRect);

        nRight  = aClipRect.right  - aClipRect.left;
        nBottom = aClipRect.bottom - aClipRect.top;

        MacSetRectRgn ( maGraphicsData.mhClipRgn,
                        nLeft,
                        nTop,
                        nRight,
                        nBottom
                      );

        if ( maGraphicsData.mhGrowRgn != NULL )
        {
            DiffRgn( maGraphicsData.mhClipRgn,
                     maGraphicsData.mhGrowRgn,
                     maGraphicsData.mhClipRgn
                   );
        } // if
    } // else

    maGraphicsData.mbClipRegionChanged = TRUE;
} // SalGraphics::ResetClipRegion

// -----------------------------------------------------------------------

void SalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
    #pragma unused(nRectCount)

    if ( maGraphicsData.mhClipRgn != NULL )
    {
        DisposeRgn( maGraphicsData.mhClipRgn );

        maGraphicsData.mhClipRgn = NULL;
    } // if
} // SalGraphics::BeginSetClipRegion

// -----------------------------------------------------------------------

BOOL SalGraphics::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    RgnHandle  hClipRegion        = NULL;
    BOOL       bClipRegionsJoined = FALSE;

    hClipRegion = NewRgn();

    if ( hClipRegion != NULL )
    {
        short  nLeft   = nX;
        short  nTop    = nY;
        short  nRight  = nX+nWidth;
        short  nBottom = nY+nHeight;

        MacSetRectRgn ( hClipRegion,
                        nLeft,
                        nTop,
                        nRight,
                        nBottom
                      );

        if ( maGraphicsData.mhClipRgn != NULL )
        {
            MacUnionRgn ( maGraphicsData.mhClipRgn,
                          hClipRegion,
                          maGraphicsData.mhClipRgn
                        );

            // Was there a QD error when joining clip regions?

            maGraphicsData.mnMacOSStatus = QDErr();

            DisposeRgn( hClipRegion );
        } // if
        else
        {
            maGraphicsData.mhClipRgn = hClipRegion;

            maGraphicsData.mnMacOSStatus = noErr;
        } // else

        if ( maGraphicsData.mnMacOSStatus == noErr )
        {
            bClipRegionsJoined = TRUE;
        } // if
    } // if

    return bClipRegionsJoined;
} // SalGraphics::UnionClipRegion

// -----------------------------------------------------------------------

void SalGraphics::EndSetClipRegion()
{
    if (    ( maGraphicsData.mhClipRgn != NULL )
         && ( maGraphicsData.mhGrowRgn != NULL )
       )
    {
        DiffRgn( maGraphicsData.mhClipRgn,
                 maGraphicsData.mhGrowRgn,
                 maGraphicsData.mhClipRgn
               );
    } // if
    else
    {
        ResetClipRegion();
    } // else

    maGraphicsData.mbClipRegionChanged = TRUE;
} // SalGraphics::EndSetClipRegion

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor()
{
    maGraphicsData.mbTransparentPen = TRUE;
} // SalGraphics::SetLineColor

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor( SalColor nSalColor )
{
    maGraphicsData.mbTransparentPen = FALSE;
    maGraphicsData.maPenColor       = SALColor2RGBColor( nSalColor );
} // SalGraphics::SetLineColor

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
    maGraphicsData.mbTransparentBrush = TRUE;
} // SalGraphics::SetFillColor

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
    RGBColor aRGBColor;

    aRGBColor = SALColor2RGBColor( nSalColor );

    maGraphicsData.maBrushColor       = aRGBColor;
    maGraphicsData.mbTransparentBrush = FALSE;
} // SalGraphics::SetFillColor

// -----------------------------------------------------------------------

void SalGraphics::SetXORMode( BOOL bSet )
{
    if ( bSet == TRUE )
    {
        maGraphicsData.mnPenMode = patXor;
    } // if
    else
    {
        maGraphicsData.mnPenMode = patCopy;
    } // else
} // SalGraphics::SetXORMode

// -----------------------------------------------------------------------

void SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY )
{
    OSStatus aQDStatus = noErr;

    aQDStatus = OpenQDPort( &maGraphicsData );

    if ( aQDStatus == noErr )
    {
        aQDStatus = BeginClip( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            RGBColor aPixelRGBColor =  maGraphicsData.maPenColor;

            SetCPixel( nX, nY, &aPixelRGBColor );

            EndClip( &maGraphicsData );
        } // if

        CloseQDPort( &maGraphicsData );
    } // if
} // SalGraphics::DrawPixel

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor )
{
    OSStatus aQDStatus = noErr;

    aQDStatus = OpenQDPort( &maGraphicsData );

    if ( aQDStatus == noErr )
    {
        aQDStatus = BeginClip( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            RGBColor aPixelRGBColor;

            aPixelRGBColor = SALColor2RGBColor( nSalColor );

            SetCPixel( nX, nY, &aPixelRGBColor );

            EndClip( &maGraphicsData );
        } // if

        CloseQDPort( &maGraphicsData );
    } // if
} // SalGraphics::DrawPixel

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
    OSStatus aQDStatus = noErr;

    aQDStatus = OpenQDPort( &maGraphicsData );

    if ( aQDStatus == noErr )
    {
        aQDStatus = BeginClip( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            short       nPortPenMode = 0;
            short       nPenMode     = maGraphicsData.mnPenMode;
            CGrafPtr    pCGraf       = maGraphicsData.mpCGrafPort;

            // What is the current pen mode associated with this graph port?

            nPortPenMode = GetPortPenMode( pCGraf );

            SetPortPenMode( pCGraf, nPenMode );

            MoveTo( nX1, nY1 );

            if ( maGraphicsData.mbTransparentPen == TRUE )
            {
                SetBlackForeColor();
            } // if
            else
            {
                RGBColor  aPenColor = maGraphicsData.maPenColor;

                RGBForeColor( &aPenColor );
            } // else

            MacLineTo( nX2, nY2 );

            // Reset the port to its original pen mode

            SetPortPenMode( pCGraf, nPortPenMode );

            EndClip( &maGraphicsData );
        } // if

        CloseQDPort( &maGraphicsData );
    } // if
} // SalGraphics::DrawLine

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight )
{
    OSStatus aQDStatus = noErr;

    aQDStatus = OpenQDPort( &maGraphicsData );

    if ( aQDStatus == noErr )
    {
        aQDStatus = BeginClip( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            long      nEndX       = 0;
            long      nEndY       = 0;
            RGBColor  aBrushColor = maGraphicsData.maBrushColor;
            Rect      aRect;

            // Compute the second set of (nX,nY) coordinates

            nEndX = nX + nWidth;
            nEndY = nY + nHeight;

            RGBForeColor( &aBrushColor );

            MoveTo( nX, nY );

            MacSetRect( &aRect, nX, nY, nEndX, nEndY );

            if ( maGraphicsData.mbTransparentBrush == TRUE )
            {
                short       nPortPenMode = 0;
                short       nPenMode     = maGraphicsData.mnPenMode;
                CGrafPtr    pCGraf       = maGraphicsData.mpCGrafPort;

                // What is the current pen mode associated with this graph port?

                nPortPenMode = GetPortPenMode( pCGraf );

                SetPortPenMode( pCGraf, nPenMode );

                MacFrameRect( &aRect );

                // Reset the port to its original pen mode

                SetPortPenMode( pCGraf, nPortPenMode );
            } // if
            else
            {
                PaintRect( &aRect );
            } // else

            EndClip( &maGraphicsData );
        } // if

        CloseQDPort( &maGraphicsData );
    } // if
} // SalGraphics::DrawRect

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint *pPtAry )
{
    if  ( ( nPoints > 1 ) && ( pPtAry != NULL ) )
    {
        OSStatus aQDStatus = noErr;

        aQDStatus = OpenQDPort( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            aQDStatus = BeginClip( &maGraphicsData );

            if ( aQDStatus == noErr )
            {
                long        nPolyEdges   = 0;
                short       nPortPenMode = 0;
                short       nPenMode     = maGraphicsData.mnPenMode;
                CGrafPtr    pCGraf       = maGraphicsData.mpCGrafPort;
                PolyHandle  hPolygon     = NULL;

                // What is the current pen mode associated with this graph port?

                nPortPenMode = GetPortPenMode( pCGraf );

                SetBlackForeColor();

                SetPortPenMode( pCGraf, nPenMode );

                // Construct a polygon

                hPolygon = OpenPoly();

                    if ( hPolygon != NULL )
                    {
                        MoveTo ( pPtAry[0].mnX,  pPtAry[0].mnY );

                        for ( nPolyEdges = 1; nPolyEdges < nPoints; nPolyEdges ++ )
                        {
                            MacLineTo( pPtAry[nPolyEdges].mnX, pPtAry[nPolyEdges].mnY );
                        } // for

                        MacLineTo( pPtAry[0].mnX,  pPtAry[0].mnY );
                    } // if

                ClosePoly();

                // Did a QD error occur whilst constructing a polygon?

                maGraphicsData.mnMacOSStatus = QDErr();

                if ( ( maGraphicsData.mnMacOSStatus == noErr ) && ( hPolygon != NULL )  )
                {
                    FramePoly( hPolygon );

                    KillPoly( hPolygon );
                } // if

                // Reset the port to its original pen mode

                SetPortPenMode( pCGraf, nPortPenMode );

                EndClip( &maGraphicsData );
            } // if

            CloseQDPort( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawPolyLine

// -----------------------------------------------------------------------

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    if  ( ( nPoints > 1 ) && ( pPtAry != NULL ) )
    {
        OSStatus aQDStatus = noErr;

        aQDStatus = OpenQDPort( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            aQDStatus = BeginClip( &maGraphicsData );

            if ( aQDStatus == noErr )
            {
                long        nPolyEdges   = 0;
                short       nPortPenMode = 0;
                short       nPenMode     = maGraphicsData.mnPenMode;
                CGrafPtr    pCGraf       = maGraphicsData.mpCGrafPort;
                PolyHandle  hPolygon     = NULL;
                RGBColor    aPolyColor   = maGraphicsData.maBrushColor;

                // What is the current pen mode associated with this graph port?

                nPortPenMode = GetPortPenMode( pCGraf );

                RGBForeColor( &aPolyColor );

                SetPortPenMode( pCGraf, nPenMode );

                // Construct a polygon

                hPolygon = OpenPoly();

                    if ( hPolygon != NULL )
                    {
                        MoveTo ( pPtAry[0].mnX,  pPtAry[0].mnY );

                        for ( nPolyEdges = 1; nPolyEdges < nPoints; nPolyEdges ++ )
                        {
                            MacLineTo( pPtAry[nPolyEdges].mnX, pPtAry[nPolyEdges].mnY );
                        } // for

                        MacLineTo( pPtAry[0].mnX,  pPtAry[0].mnY );
                    } // if

                ClosePoly();

                // Did a QD error occur whilst constructing a polygon?

                maGraphicsData.mnMacOSStatus = QDErr();

                if ( ( maGraphicsData.mnMacOSStatus == noErr ) && ( hPolygon != NULL )  )
                {
                    PaintPoly( hPolygon );

                    KillPoly( hPolygon );
                } // if

                // Reset the port to its original pen mode

                SetPortPenMode( pCGraf, nPortPenMode );

                EndClip( &maGraphicsData );
            } // if

            CloseQDPort( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawPolygon

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG            nPoly,
                                   const ULONG     *pPoints,
                                   PCONSTSALPOINT  *ppPtAry
                                 )
{
    if  ( ( nPoly > 1 ) && ( ppPtAry != NULL ) && ( ppPtAry != NULL ) )
    {
        OSStatus aQDStatus = noErr;

        aQDStatus = OpenQDPort( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            aQDStatus = BeginClip( &maGraphicsData );

            if ( aQDStatus == noErr )
            {
                short       nPortPenMode = 0;
                short       nPenMode     = maGraphicsData.mnPenMode;
                CGrafPtr    pCGraf       = maGraphicsData.mpCGrafPort;
                RgnHandle   hPolyRgn     = NULL;
                RGBColor    aPolyColor   = maGraphicsData.maBrushColor;
                OSStatus    aQDStatus    = noErr;

                // What is the current pen mode associated with this graph port?

                nPortPenMode = GetPortPenMode( pCGraf );

                RGBForeColor( &aPolyColor );

                SetPortPenMode( pCGraf, nPenMode );

                // Construct a polygon region

                hPolyRgn = GetPolygonRgn(  nPoly,
                                           pPoints,
                                           ppPtAry,
                                          &aQDStatus
                                        );

                if ( ( hPolyRgn != NULL ) && ( aQDStatus == noErr ) )
                {
                    MacPaintRgn( hPolyRgn );

                    DisposeRgn( hPolyRgn );

                    aQDStatus = QDErr();

                    hPolyRgn = NULL;
                } // if

                // Reset the port to its original pen mode

                SetPortPenMode( pCGraf, nPortPenMode );

                maGraphicsData.mnMacOSStatus = aQDStatus;

                EndClip( &maGraphicsData );
            } // if

            CloseQDPort( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawPolyPolygon

// -----------------------------------------------------------------------

void SalGraphics::CopyBits( const SalTwoRect  *pPosAry,
                            SalGraphics       *pSrcGraphics
                          )
{
    if (    ( maGraphicsData.mpCGrafPort != NULL )
         && ( pPosAry                    != NULL )
         && ( pSrcGraphics               != NULL )
       )
    {
        OSStatus aQDStatus = noErr;

        aQDStatus = OpenQDPort( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            aQDStatus = BeginClip( &maGraphicsData );

            if ( aQDStatus == noErr )
            {
                const BitMap  *pDstBitMap = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );

                if ( pDstBitMap != NULL )
                {
                    maGraphicsData.mnMacOSStatus = LockPortBits( maGraphicsData.mpCGrafPort );

                    if ( maGraphicsData.mnMacOSStatus == noErr )
                    {
                        Rect       aSrcRect;
                        Rect       aDstRect;
                        Rect       aPortBoundsRect;
                        RgnHandle  hMaskRgn  = NULL;  // Mask Region for QD CopyBits
                        short      nCopyMode = 0;

                        SalTwoRect2SQDSrcRect( pPosAry, &aSrcRect );

                        SalTwoRect2SQDDstRect( pPosAry, &aDstRect );

                        GetPortBounds( pSrcGraphics->maGraphicsData.mpCGrafPort, &aPortBoundsRect );

                        CheckRectBounds( &aSrcRect, &aDstRect, &aPortBoundsRect );

                        nCopyMode = SelectCopyMode( &maGraphicsData );

                        // Now we can call QD CopyBits to copy the bits from source rectangle
                        // to the destination rectangle

                        if (    ( pSrcGraphics != NULL )
                             && ( pSrcGraphics->maGraphicsData.mpCGrafPort != NULL )
                           )
                        {
                            const BitMap  *pSrcBitMap
                                = GetPortBitMapForCopyBits
                                    ( pSrcGraphics->maGraphicsData.mpCGrafPort );

                            if ( pSrcBitMap != NULL )
                            {
                                ::CopyBits (  pSrcBitMap,
                                              pDstBitMap,
                                             &aSrcRect,
                                             &aDstRect,
                                              nCopyMode,
                                              hMaskRgn
                                           );
                            } // if
                        } // if
                        else
                        {
                            ::CopyBits (  pDstBitMap,
                                          pDstBitMap,
                                         &aSrcRect,
                                         &aDstRect,
                                          nCopyMode,
                                          hMaskRgn
                                       );
                        } // else

                        UnlockPortBits( maGraphicsData.mpCGrafPort );
                    } // if
                } // if

                EndClip( &maGraphicsData );
            } // if

            CloseQDPort( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::CopyBits

// -----------------------------------------------------------------------

void SalGraphics::CopyArea( long    nDstX,
                            long    nDstY,
                            long    nSrcX,
                            long    nSrcY,
                            long    nSrcWidth,
                            long    nSrcHeight,
                            USHORT  nFlags
                          )
{
    #pragma unused(nFlags)

    if ( maGraphicsData.mpCGrafPort != NULL )
    {
        OSStatus aQDStatus = noErr;

        aQDStatus = OpenQDPort( &maGraphicsData );

        if ( aQDStatus == noErr )
        {
            aQDStatus = BeginClip( &maGraphicsData );

            if ( aQDStatus == noErr )
            {
                const BitMap  *pSrcBitMap = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );
                const BitMap  *pDstBitMap = pSrcBitMap;

                if ( pSrcBitMap != NULL )
                {
                    maGraphicsData.mnMacOSStatus = LockPortBits( maGraphicsData.mpCGrafPort );

                    if ( maGraphicsData.mnMacOSStatus == noErr )
                    {
                        short      nCopyMode  = 0;
                        long       nDstWidth  = nSrcWidth;
                        long       nDstHeight = nSrcHeight;
                        RgnHandle  hMaskRgn   = NULL;  // Mask Region for QD CopyBits
                        Rect       aSrcRect;
                        Rect       aDstRect;
                        Rect       aPortBoundsRect;

                        GetQDRect ( nSrcX, nSrcY, nSrcWidth, nSrcHeight, &aSrcRect );

                        GetQDRect ( nDstX, nDstY, nDstWidth, nDstHeight, &aDstRect );

                        GetPortBounds( maGraphicsData.mpCGrafPort, &aPortBoundsRect );

                        CheckRectBounds( &aSrcRect, &aDstRect, &aPortBoundsRect );

                        nCopyMode = SelectCopyMode( &maGraphicsData );

                        // Now we can call QD CopyBits to copy the bits from source rectangle
                        // to the destination rectangle

                        ::CopyBits (  pSrcBitMap,
                                      pDstBitMap,
                                     &aSrcRect,
                                     &aDstRect,
                                      nCopyMode,
                                      hMaskRgn
                                   );

                        UnlockPortBits( maGraphicsData.mpCGrafPort );
                    } // if
                } // if

                EndClip( &maGraphicsData );
            } // if

            CloseQDPort( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::CopyArea

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              SalColor nTransparentColor )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransparentBitmap )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawMask( const SalTwoRect* pPosAry,
                            const SalBitmap& rSalBitmap,
                            SalColor nMaskColor )
{
} // SalGraphics::DrawMask

// -----------------------------------------------------------------------

SalBitmap* SalGraphics::GetBitmap( long nX, long nY, long nDX, long nDY )
{
    return NULL;
}

// -----------------------------------------------------------------------

SalColor SalGraphics::GetPixel( long nX, long nY )
{
    RGBColor  aRGBColor;
    SalColor  nSalColor       = 0;
    short     aHorizontalCoor = (short)nX;
    short     aVerticalCoor   = (short)nY;

    GetCPixel( aHorizontalCoor, aVerticalCoor, &aRGBColor );

    nSalColor = RGBColor2SALColor ( &aRGBColor );

    return nSalColor;
} // SalGraphics::GetPixel

// -----------------------------------------------------------------------

void SalGraphics::Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
}

// -----------------------------------------------------------------------

void SalGraphics::Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nSalFlags )
{
}

// -----------------------------------------------------------------------

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight,
              void* pPtr, ULONG nSize )
{
    return FALSE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetTextColor( SalColor nSalColor )
{
    maGraphicsData.maFontColor = SALColor2RGBColor( nSalColor );
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont )
{
    return 0;
}

// -----------------------------------------------------------------------

long SalGraphics::GetCharWidth( sal_Unicode nChar1, sal_Unicode nChar2, long* pWidthAry )
{
    // Stub code: we have not yet written any interfaces to native fonts.
    // However, we need to get some font info in order to continue porting.
    return 10;
}

// -----------------------------------------------------------------------

void SalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
    // Stub Code

    FMetricRec aFMetric;

    FontMetrics( &aFMetric );

    pMetric->mnAscent = 10;
    pMetric->mnDescent = 10;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
    return 0;
}

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontList( ImplDevFontList* pList )
{
    // Stub code: we have not yet written any interfaces to native fonts.
    // However, we need to create at least one fake font to continue our
    // porting.
    ImplFontData *pFontData = new ImplFontData;
    pFontData->mnWidth = 10;
    pFontData->mnHeight = 10;
    pList->Add( pFontData );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawText( long                nX,
                            long                nY,
                            const xub_Unicode  *pStr,
                            xub_StrLen          nLen
                          )
{
    // The implementation is not yet complete

    if ( ( pStr != NULL ) && ( nLen > 0 ) )
    {
        short        nFirstByte = 0;
        short        nByteCount = nLen;
        const char  *pTextBuf   = (char *)pStr;

        MoveTo( nX, nY );

        ::MacDrawText( pTextBuf, nFirstByte, nByteCount );
    } // if
} // SalGraphics::DrawText

// -----------------------------------------------------------------------

void SalGraphics::DrawTextArray( long nX, long nY,
                                 const xub_Unicode* pStr, xub_StrLen nLen,
                                 const long* pDXAry )
{
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( xub_Unicode cChar, long* pX, long* pY,
                                     long* pWidth, long* pHeight )
{
    return FALSE;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetGlyphOutline( xub_Unicode cChar, USHORT** ppPolySizes,
                                    SalPoint** ppPoints, BYTE** ppFlags )
{
    return 0;
}

// =======================================================================

