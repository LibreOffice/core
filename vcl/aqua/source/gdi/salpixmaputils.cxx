/*************************************************************************
 *
 *  $RCSfile: salpixmaputils.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: bmahbod $ $Date: 2001-03-12 23:15:32 $
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

#define _SV_SALPIXMAPUTILS_CXX

#ifndef _SV_SALPIXMAPUTILS_HXX
    #include <salpixmaputils.hxx>
#endif

// =======================================================================

// =======================================================================

static inline long GetPixMapHeight( const Rect *rPixMapBoundsRect )
{
    long  nPixMapHeight = rPixMapBoundsRect->bottom - rPixMapBoundsRect->top;

    return nPixMapHeight;
} // GetPixMapHeight

// ------------------------------------------------------------------

static inline long GetPixMapWidth ( const Rect *rPixMapBoundsRect )
{
    long  nPixMapWidth = rPixMapBoundsRect->right - rPixMapBoundsRect->left;

    return nPixMapWidth;
} // GetPixMapWidth

// ------------------------------------------------------------------

static inline Fixed GetPixMapRes( const long nPixMapRes )
{
    Fixed aPixMapRes = Long2Fix( nPixMapRes );

    return aPixMapRes;
} // GetPixMapHRes

// ------------------------------------------------------------------

static inline long GetPixMapBitDepth( const USHORT nPixMapBits )
{
    long nPixMapBitDepth = 0;

    if ( nPixMapBits <= kThousandsColor )
    {
        nPixMapBitDepth = (long)nPixMapBits;
    } // if
    else
    {
        nPixMapBitDepth = kTrueColor;
    } // else

    return nPixMapBitDepth;
} // GetNewPixMapDepth

// ------------------------------------------------------------------

static inline long GetPixMapOffset ( const long   nPixMapBitDepth,
                                     const short  nPixMapWidth
                                   )
{
    long  nPixMapImageWidth = nPixMapBitDepth * (long)nPixMapWidth;
    long  nPixMapOffset     = ( ( nPixMapImageWidth + 15L ) >> 4L ) << 1L;

    return nPixMapOffset;
} // GetPixMapOffset

// ------------------------------------------------------------------

static inline long GetPixMapImageSize ( const short  nPixMapHeight,
                                        const long   nPixMapRowOffset
                                      )
{
    long nPixMapImageSize = (long)nPixMapHeight * nPixMapRowOffset;

    return nPixMapImageSize;
} // GetPixMapImageSize

// ------------------------------------------------------------------

static inline short GetPixMapRowBytes ( const long nPixMapRowOffset )
{
    short nPixMapRowBytes = (short)nPixMapRowOffset + 0x8000;

    return nPixMapRowBytes;
} // GetPixMapRowBytes

// ------------------------------------------------------------------

static inline short GetPixMapColorDepth ( const USHORT nPixMapBits )
{
    short nPixMapColorDepth = 0;

    if ( nPixMapBits <= kEightBits )
    {
        nPixMapColorDepth = kOneBit << ((short)nPixMapBits);
    }

    return nPixMapColorDepth;
} // GetPixMapColorDepth

// ------------------------------------------------------------------

static void GetPixMapBoudsRect ( const short   nPixMapWidth,
                                 const short   nPixMapHeight,
                                 Rect         *rPixMapBoundsRect
                               )
{
    short  nPixMapRectLeft   = 0;
    short  nPixMapRectTop    = 0;
    short  nPixMapRectRight  = nPixMapWidth;
    short  nPixMapRectBottom = nPixMapHeight;

    // Set the dimensions of the PixMap

    MacSetRect( rPixMapBoundsRect,
                nPixMapRectLeft,
                nPixMapRectTop,
                nPixMapRectRight,
                nPixMapRectBottom
              );
} // GetPixMapBoudsRect

// ------------------------------------------------------------------

static inline short GetPixMapCmpSize ( const long nPixMapBitDepth )
{
    short nPixMapCmpSize = 0;

    if ( nPixMapBitDepth <= kEightBitColor )
    {
        nPixMapCmpSize = nPixMapBitDepth;
    } // if
    else if ( nPixMapBitDepth == kThousandsColor )
    {
        nPixMapCmpSize = kPixMapCmpSizeFiveBits;
    } // else if
    else
    {
        nPixMapCmpSize = kPixMapCmpSizeEightBits;
    } // else

    return nPixMapCmpSize;
} // GetPixMapCmpSize

// ------------------------------------------------------------------

static inline short GetPixMapCmpCount ( const long nPixMapBitDepth )
{
    short nPixMapCmpCount = 0;

    if ( nPixMapBitDepth <= kEightBitColor )
    {
        nPixMapCmpCount = 1;
    } // if
    else
    {
        nPixMapCmpCount = 3;
    } // else

    return nPixMapCmpCount;
} // GetPixMapCmpCount

// ------------------------------------------------------------------

static inline short GetPixMapPixelType ( const long nPixMapBitDepth )
{
    short nPixMapPixelType = 0;

    if ( nPixMapBitDepth <= kEightBitColor )
    {
        nPixMapPixelType = 0;
    } // if
    else
    {
        nPixMapPixelType = RGBDirect;
    } // else

    return nPixMapPixelType;
} // GetPixMapPixelType

// ------------------------------------------------------------------

static inline OSType GetPixMapPixelFormat ( )
{
    OSType  nPixMapPixelFormat = NULL;
    GDPtr   pGDevice           = NULL;

    pGDevice = *GetGDevice( );

    if ( pGDevice != NULL )
    {
        PixMapPtr  pPixMap = NULL;

        pPixMap = *(*pGDevice).gdPMap;

        if ( pPixMap != NULL )
        {
            nPixMapPixelFormat = pPixMap->pixelFormat;
        } // if
    } // if

    return nPixMapPixelFormat;
} // GetPixMapPixelFormat

// =======================================================================

// =======================================================================

static CTabHandle GetPixMapCTabBitmapPalette ( const short           nPixMapColorDepth,
                                               const BitmapPalette  &rBitmapPalette
                                             )
{
    CTabHandle  hPixMapCTab = NULL;

    hPixMapCTab = (CTabHandle) NewHandleClear(    sizeof( ColorTable )
                                                + sizeof( ColorSpec )
                                                * ( nPixMapColorDepth - 1 )
                                             );

    if ( ( hPixMapCTab != NULL ) && ( *hPixMapCTab != NULL ) )
    {
        SInt8 nFlags = noErr;

        nFlags = HGetState( (Handle)hPixMapCTab );

        if ( nFlags == noErr )
        {
            short  nBitmapPaletteMinCount = GetMinColorCount(nPixMapColorDepth, rBitmapPalette);
            short  nBitmapPaletteIndex    = 0;

            HLock( (Handle)hPixMapCTab );

                (**hPixMapCTab).ctSeed  = GetCTSeed();
                (**hPixMapCTab).ctFlags = 0;
                (**hPixMapCTab).ctSize  = nPixMapColorDepth - 1;

                for ( nBitmapPaletteIndex = 0;
                      nBitmapPaletteIndex < nBitmapPaletteMinCount;
                      nBitmapPaletteIndex++
                    )
                {
                    const BitmapColor  &rBitmapPaletteColor = rBitmapPalette[nBitmapPaletteIndex];

                    (**hPixMapCTab).ctTable[nBitmapPaletteIndex].value
                        = nBitmapPaletteIndex;

                    (**hPixMapCTab).ctTable[nBitmapPaletteIndex].rgb
                        = BitmapColor2RGBColor( rBitmapPaletteColor );
                } // for

            HSetState( (Handle)hPixMapCTab, nFlags );
        } // if
    } // if

    return hPixMapCTab;
} // GetPixMapCTabBitmapPalette

// ------------------------------------------------------------------

static CTabHandle GetCTabRGBDirect ( const short nPixMapCmpSize )
{
    CTabHandle  hPixMapCTab = NULL;

    hPixMapCTab = (CTabHandle)NewHandleClear(   sizeof( ColorTable )
                                              - sizeof( CSpecArray )
                                            );

    if ( ( hPixMapCTab != NULL ) && ( *hPixMapCTab != NULL ) )
    {
        SInt8  nFlags = noErr;

        nFlags = HGetState( (Handle)hPixMapCTab );

        if ( nFlags == noErr )
        {
            HLock( (Handle)hPixMapCTab );

                (**hPixMapCTab).ctSeed  = 3 * nPixMapCmpSize;
                (**hPixMapCTab).ctFlags = 0;
                (**hPixMapCTab).ctSize  = 0;

            HSetState( (Handle)hPixMapCTab, nFlags );
        } // if
    } // if

    return hPixMapCTab;
} // GetCTabRGBDirect

// ------------------------------------------------------------------

static CTabHandle GetPixMapCTab ( const long            nPixMapBitDepth,
                                  const short           nPixMapColorDepth,
                                  const short           nPixMapCmpSize,
                                  const BitmapPalette  &rBitmapPalette
                                )
{
    CTabHandle hPixMapCTab = NULL;

    if ( nPixMapBitDepth <= kEightBitColor )
    {
        hPixMapCTab = GetPixMapCTabBitmapPalette( nPixMapColorDepth,
                                                  rBitmapPalette
                                                );

        if ( hPixMapCTab == NULL )
        {
            hPixMapCTab = GetCTabFromStdCLUT( nPixMapBitDepth );

            if ( hPixMapCTab == NULL )
            {
                hPixMapCTab = CopyGDeviceCTab( );
            } // if
        } // if
    } // if
    else
    {
        hPixMapCTab = GetCTabRGBDirect( nPixMapCmpSize );
    } // else

    return hPixMapCTab;
} // GetPixMapCTab

// =======================================================================

// =======================================================================

PixMapHandle GetNewPixMap ( const Size           &rPixMapSize,
                            const USHORT          nPixMapBits,
                            const BitmapPalette  &rBitmapPalette
                          )
{
    PixMapHandle  hPixMap       = NULL;
    short         nPixMapWidth  = rPixMapSize.Width();
    short         nPixMapHeight = rPixMapSize.Height();

    if ( ( nPixMapWidth > 0 ) && ( nPixMapHeight > 0 ) )
    {
        hPixMap = NewPixMap();

        if ( ( hPixMap != NULL ) && ( *hPixMap != NULL ) )
        {
            const long   nPixMapBitDepth  = GetPixMapBitDepth( nPixMapBits);
            const long   nPixMapRowOffset = GetPixMapOffset( nPixMapBitDepth, nPixMapWidth );
            const long   nPixMapImageSize = GetPixMapImageSize( nPixMapHeight, nPixMapRowOffset );
            char        *pPixMapData      = NewPtrClear( nPixMapImageSize );

            if ( pPixMapData != NULL )
            {
                GWorldFlags  nPixMapFlags = noErr;

                nPixMapFlags = GetPixelsState( hPixMap );

                if ( nPixMapFlags == noErr )
                {
                    if ( LockPixels( hPixMap ) )
                    {
                        const short   nPixMapRowBytes    = GetPixMapRowBytes( nPixMapRowOffset );
                        const short   nPixMapColorDepth  = GetPixMapColorDepth( nPixMapBits );
                        const short   nPixMapCmpSize     = GetPixMapCmpSize( nPixMapBitDepth );
                        const short   nPixMapCmpCount    = GetPixMapCmpCount( nPixMapBitDepth );
                        const short   nPixMapPixelType   = GetPixMapPixelType( nPixMapBitDepth );
                        const OSType  aPixMapPixelFormat = GetPixMapPixelFormat( );
                        const Fixed   aPixMapHRes        = GetPixMapRes( kPixMapHRes );
                        const Fixed   aPixMapVRes        = GetPixMapRes( kPixMapVRes );
                        Rect          aPixMapBoundsRect;

                        GetPixMapBoudsRect( nPixMapWidth, nPixMapHeight, &aPixMapBoundsRect );

                        (**hPixMap).baseAddr    = pPixMapData;         // Pointer to pixels
                        (**hPixMap).rowBytes    = nPixMapRowBytes;     // Offset to next line
                        (**hPixMap).bounds      = aPixMapBoundsRect;   // Bounding bitmap rectangle
                        (**hPixMap).pmVersion   = 0;                   // PixMap version number
                        (**hPixMap).packType    = 0;                   // Defines packing format
                        (**hPixMap).packSize    = 0;                   // Length of pixel data
                        (**hPixMap).hRes        = aPixMapHRes;         // Horizontal resolution (ppi)
                        (**hPixMap).vRes        = aPixMapVRes;         // Vertical resolution (ppi)
                        (**hPixMap).pixelType   = nPixMapPixelType;    // Defines pixel type
                        (**hPixMap).pixelSize   = nPixMapBitDepth;     // Number of bits in a pixel
                        (**hPixMap).cmpCount    = nPixMapCmpCount;     // Number of components in a pixel
                        (**hPixMap).cmpSize     = nPixMapCmpSize;      // Number of bits per component
                        (**hPixMap).pixelFormat = aPixMapPixelFormat;  // Four character code representation
                        (**hPixMap).pmExt       = NULL;                // Handle to PixMap extension

                        // Get the color table based on the desired screen depth

                        (**hPixMap).pmTable = GetPixMapCTab( nPixMapBitDepth,
                                                                         nPixMapColorDepth,
                                                             nPixMapCmpSize,
                                                             rBitmapPalette
                                                           );

                        SetPixelsState( hPixMap, nPixMapFlags );
                    } // if
                    else
                    {
                        DisposePixMap( hPixMap );

                        hPixMap = NULL;
                    } // else
                } // if
                else
                {
                    DisposePixMap( hPixMap );

                    hPixMap = NULL;
                } // else
            } // if
            else
            {
                DisposePixMap( hPixMap );

                hPixMap = NULL;
            } // else
        } // if
    } // if

    return hPixMap;
} // GetNewPixMap

// =======================================================================

// =======================================================================

PixMapHandle CopyPixMap ( PixMapHandle  hPixMap )
{
    PixMapHandle  hPixMapCopy  = NULL;
    GWorldFlags   nPixMapFlags = noErr;

    nPixMapFlags = GetPixelsState( hPixMap );

    if ( nPixMapFlags == noErr )
    {
        if ( LockPixels( hPixMap ) )
        {
            const char *pPixMapData = (**hPixMap).baseAddr;

            if ( pPixMapData != NULL )
            {
                hPixMapCopy = NewPixMap();

                if (    (  hPixMapCopy != NULL )
                     && ( *hPixMapCopy != NULL )
                   )
                {
                    hPixMapCopy = NewPixMap();

                    if (    (  hPixMapCopy != NULL )
                         && ( *hPixMapCopy != NULL )
                       )
                    {
                        const Rect   aPixMapBoundsRect = (**hPixMap).bounds;
                        const long   nPixMapBitDepth   = (**hPixMap).pixelSize;
                        const long   nPixMapWidth      = GetPixMapWidth( &aPixMapBoundsRect );
                        const long   nPixMapHeight     = GetPixMapHeight( &aPixMapBoundsRect );
                        const long   nPixMapRowOffset  = GetPixMapOffset( nPixMapBitDepth, nPixMapWidth );
                        const long   nPixMapImageSize  = GetPixMapImageSize( nPixMapHeight, nPixMapRowOffset );
                        char        *pPixMapDataCopy   = NewPtrClear( nPixMapImageSize );

                        if ( pPixMapDataCopy != NULL )
                        {
                            GWorldFlags  nPixMapCopyFlags = noErr;

                            nPixMapCopyFlags = GetPixelsState( hPixMapCopy );

                            if ( nPixMapCopyFlags == noErr )
                            {
                                if ( LockPixels( hPixMapCopy ) )
                                {

                                    BlockMoveData( pPixMapData, pPixMapDataCopy, nPixMapImageSize );

                                    (**hPixMapCopy).rowBytes    = (**hPixMap).rowBytes;     // Offset to next line
                                    (**hPixMapCopy).bounds      = (**hPixMap).bounds;       // Bounding bitmap rectangle
                                    (**hPixMapCopy).pmVersion   = (**hPixMap).pmVersion;    // PixMap version number
                                    (**hPixMapCopy).packType    = (**hPixMap).packType;     // Defines packing format
                                    (**hPixMapCopy).packSize    = (**hPixMap).packSize;     // Length of pixel data
                                    (**hPixMapCopy).hRes        = (**hPixMap).hRes;         // Horizontal resolution (ppi)
                                    (**hPixMapCopy).vRes        = (**hPixMap).vRes;         // Vertical resolution (ppi)
                                    (**hPixMapCopy).pixelType   = (**hPixMap).pixelType;    // Defines pixel type
                                    (**hPixMapCopy).pixelSize   = (**hPixMap).pixelSize;    // Number of bits in a pixel
                                    (**hPixMapCopy).cmpCount    = (**hPixMap).cmpCount;     // Number of components in a pixel
                                    (**hPixMapCopy).cmpSize     = (**hPixMap).cmpSize;      // Number of bits per component
                                    (**hPixMapCopy).pixelFormat = (**hPixMap).pixelFormat;  // Four character code representation
                                    (**hPixMapCopy).pmExt       = (**hPixMap).pmExt;        // Handle to PixMap extension

                                    // Copy the color table from the original port

                                    (**hPixMapCopy).pmTable = CopyPixMapCTab( hPixMap );

                                    SetPixelsState( hPixMapCopy, nPixMapCopyFlags );
                                } // if
                                else
                                {
                                    DisposePixMap( hPixMapCopy );

                                    hPixMapCopy = NULL;
                                } // else
                            } // if
                            else
                            {
                                DisposePixMap( hPixMapCopy );

                                hPixMapCopy = NULL;
                            } // else
                        } // if
                        else
                        {
                            DisposePixMap( hPixMapCopy );

                            hPixMapCopy = NULL;
                        } // else
                    } // if
                } // if
            } // if

            SetPixelsState( hPixMap, nPixMapFlags );
        } // if
    } // if

    return hPixMapCopy;
} // CopyPixMap

// =======================================================================

// =======================================================================

PixMapHandle GetCGrafPortPixMap ( const Size           &rPixMapSize,
                                  const USHORT          nPixMapBits,
                                  const BitmapPalette  &rBitmapPalette,
                                  const CGrafPtr        pCGraf
                                )
{
    PixMapHandle hNewPixMap = GetPortPixMap( pCGraf );

    if ( hNewPixMap == NULL )
    {
        hNewPixMap = GetNewPixMap( rPixMapSize,
                                           nPixMapBits,
                                           rBitmapPalette
                                         );

        if ( hNewPixMap == NULL )
        {
            GDHandle hGDevice = GetGDevice( );

            if ( ( hGDevice != NULL ) && ( *hGDevice != NULL ) )
            {
                SInt8  nGDeviceFlags = noErr;

                nGDeviceFlags = HGetState( (Handle)hGDevice );

                if ( nGDeviceFlags == noErr )
                {
                    PixMapHandle hPixMap = NULL;

                    HLock( (Handle)hGDevice );

                        hPixMap = (**hGDevice).gdPMap;

                        if ( ( hPixMap != NULL ) && ( *hPixMap != NULL ) )
                        {
                            hNewPixMap = CopyPixMap( hPixMap );
                        } // if

                    HSetState( (Handle)hGDevice, nGDeviceFlags );
                } //if
            } // if
        } // if
    } // if

    return hNewPixMap;
} // GetCGrafPortPixMap

// =======================================================================

// =======================================================================
