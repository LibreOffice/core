/*************************************************************************
 *
 *  $RCSfile: salbmp.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: bmahbod $ $Date: 2001-01-31 23:56:13 $
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
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

// ==================================================================

static inline long GetNewPixMapBitDepth( const USHORT nPixMapBits )
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

static inline long GetNewPixMapOffset( const long   nPixMapBitDepth,
                                       const short  nPixMapWidth
                                     )
{
    long  nPixMapImageWidth = nPixMapBitDepth * (long)nPixMapWidth;
    long  nPixMapOffset     = ( ( nPixMapImageWidth + 15L ) >> 4L ) << 1L;

    return nPixMapOffset;
} // GetNewPixMapOffset

// ------------------------------------------------------------------

static inline long GetNewPixMapImageSize ( const short  nPixMapHeight,
                                           const long   nPixMapRowOffset
                                         )
{
    long nPixMapImageSize = (long)nPixMapHeight * nPixMapRowOffset;

    return nPixMapImageSize;
} // GetNewPixMapImageSize

// ------------------------------------------------------------------

static inline short GetNewPixMapRowBytes( const long nPixMapRowOffset )
{
    short nPixMapRowBytes = (short)nPixMapRowOffset + 0x8000;

    return nPixMapRowBytes;
} // GetNewPixMapRowBytes

// ------------------------------------------------------------------

static inline short GetNewPixMapColorDepth( const USHORT nPixMapBits )
{
    short nPixMapColorDepth = 0;

    if ( nPixMapBits <= 8 )
    {
        nPixMapColorDepth = 1 << ((short)nPixMapBits);
    }

    return nPixMapColorDepth;
} // GetNewPixMapColorDepth

// ------------------------------------------------------------------

static void GetNewPixMapBoudsRect( const short   nPixMapWidth,
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
} // GetNewPixMapBoudsRect

// ------------------------------------------------------------------

static inline short GetNewPixMapCmpSize( const long nPixMapBitDepth )
{
    short nPixMapCmpSize = 0;

    if ( nPixMapBitDepth <= kEightBitColor )
    {
        nPixMapCmpSize = nPixMapBitDepth;
    } // if
    else if ( nPixMapBitDepth == kThousandsColor )
    {
        nPixMapCmpSize = 5;
    } // else if
    else
    {
        nPixMapCmpSize = 8;
    } // else

    return nPixMapCmpSize;
} // GetNewPixMapCmpSize

// ------------------------------------------------------------------

static inline short GetNewPixMapCmpCount( const long nPixMapBitDepth )
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
} // GetNewPixMapCmpCount

// ------------------------------------------------------------------

static inline short GetNewPixMapPixelType( const long nPixMapBitDepth )
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
} // GetNewPixMapPixelType

// ------------------------------------------------------------------

static inline OSType GetNewPixMapPixelFormat( )
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
} // GetNewPixMapPixelFormat

// ------------------------------------------------------------------

static CTabHandle CopyGDeviceCTab( )
{
    GDPtr       pGDevice = NULL;
    CTabHandle  hCTable  = NULL;

    pGDevice = *GetGDevice ( );

    if ( pGDevice != NULL )
    {
        PixMapPtr  pPixMap = NULL;

        pPixMap = *(*pGDevice).gdPMap;

        if ( pPixMap != NULL )
        {
            CTabPtr pCTable = NULL;

            pCTable = *(*pPixMap).pmTable;

            if ( pCTable != NULL )
            {
                const short  nCTableSize       = pCTable->ctSize + 1;
                long         nCTableHandleSize = 0;

                nCTableHandleSize =   (   nCTableSize
                                        * sizeof( ColorSpec )
                                      )
                                    + (   sizeof( ColorTable )
                                        - sizeof( CSpecArray )
                                      );

                hCTable = (CTabHandle) NewHandleClear( nCTableHandleSize );

                if ( ( hCTable != NULL ) && ( *hCTable != NULL ) )
                {
                    SInt8  nFlags = noErr;

                    nFlags = HGetState( (Handle)hCTable );

                    if ( nFlags == noErr )
                    {
                        unsigned long  nCTableIndex;

                        HLock( (Handle)hCTable );

                            (**hCTable).ctSize  = pCTable->ctSize;
                            (**hCTable).ctFlags = pCTable->ctFlags;
                            (**hCTable).ctSeed  = GetCTSeed();

                            for ( nCTableIndex = 0;
                                  nCTableIndex < nCTableSize;
                                  nCTableIndex++
                                )
                            {
                                (**hCTable).ctTable[nCTableIndex]
                                    = pCTable->ctTable[nCTableIndex];
                            } // for

                          HSetState( (Handle)hCTable, nFlags );
                      } // if
                  } // if
            } // if
        } // if
    } // if

    return hCTable;
} // CopyGDeviceCTab

// ------------------------------------------------------------------

static CTabHandle GetNewPixMapCTabCLUT( const short nPixMapBitDepth )
{
    CTabHandle  hPixMapCTab   = NULL;
    short       nPixMapCLUTID = 0;

    nPixMapCLUTID = nPixMapBitDepth + 64;

    hPixMapCTab = GetCTable( nPixMapCLUTID );

    return hPixMapCTab;
} // GetNewPixMapCTabCLUT

// ------------------------------------------------------------------

static inline short GetMinColorCount( const short           nPixMapColorDepth,
                                      const BitmapPalette&  rBitmapPalette
                                    )
{
    short nBitmapPaletteCount    = rBitmapPalette.GetEntryCount();
    short nBitmapPaletteMinCount = 0;

    if ( nPixMapColorDepth < nBitmapPaletteCount )
    {
        nBitmapPaletteMinCount = nPixMapColorDepth;
    } // if
    else
    {
        nBitmapPaletteMinCount = nBitmapPaletteCount;
    } // else

    return nBitmapPaletteMinCount;
} // GetMinColorCount

// ------------------------------------------------------------------

static CTabHandle GetNewPixMapCTabBitmapPalette( const short           nPixMapColorDepth,
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
            short  nBitmapPaletteColorVal = 0;
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

                nBitmapPaletteColorVal
                    = rBitmapPaletteColor.GetRed();

                (**hPixMapCTab).ctTable[nBitmapPaletteIndex].rgb.red
                    = ( nBitmapPaletteColorVal << 8 ) | nBitmapPaletteColorVal;

                nBitmapPaletteColorVal
                    = rBitmapPaletteColor.GetGreen();

                (**hPixMapCTab).ctTable[nBitmapPaletteIndex].rgb.green
                    = ( nBitmapPaletteColorVal << 8 ) | nBitmapPaletteColorVal ;

                nBitmapPaletteColorVal
                    = rBitmapPaletteColor.GetBlue();

                (**hPixMapCTab).ctTable[nBitmapPaletteIndex].rgb.blue
                    = ( nBitmapPaletteColorVal << 8 ) | nBitmapPaletteColorVal;
            } // for

            HSetState( (Handle)hPixMapCTab, nFlags );
        } // if
    } // if

    return hPixMapCTab;
} // GetNewPixMapCTabBitmapPalette

// ------------------------------------------------------------------

static CTabHandle GetNewPixMapCTabRGBDirect( const short nPixMapCmpSize )
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
} // GetNewPixMapCTabRGBDirect

// ------------------------------------------------------------------

static CTabHandle GetNewPixMapCTab( const long            nPixMapBitDepth,
                                    const short           nPixMapColorDepth,
                                    const short           nPixMapCmpSize,
                                    const BitmapPalette  &rBitmapPalette
                                  )
{
    CTabHandle hPixMapCTab = NULL;

    if ( nPixMapBitDepth <= kEightBitColor )
    {
        hPixMapCTab = GetNewPixMapCTabBitmapPalette( nPixMapColorDepth,
                                                     rBitmapPalette
                                                   );

        if ( hPixMapCTab == NULL )
        {
            hPixMapCTab = GetNewPixMapCTabCLUT( nPixMapBitDepth );

            if ( hPixMapCTab == NULL )
            {
                hPixMapCTab = CopyGDeviceCTab( );
            } // if
        } // if
    } // if
    else
    {
        hPixMapCTab = GetNewPixMapCTabRGBDirect( nPixMapCmpSize );
    } // else

    return hPixMapCTab;
} // GetNewPixMapCTab

// ------------------------------------------------------------------

static PixMapHandle GetNewPixMap ( const Size           &rPixMapSize,
                                   const USHORT          nPixMapBits,
                                   const BitmapPalette  &rBitmapPalette,
                                   const SalGraphics    *rSalGraphics
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
            const long  nPixMapBitDepth  = GetNewPixMapBitDepth( nPixMapBits);
            const long  nPixMapRowOffset = GetNewPixMapOffset( nPixMapBitDepth, nPixMapWidth );
            const long  nPixMapImageSize = GetNewPixMapImageSize( nPixMapHeight, nPixMapRowOffset );
            Ptr         pPixMapData      = NewPtrClear( nPixMapImageSize );

            if ( pPixMapData != NULL )
            {
                GWorldFlags  nPixMapFlags = noErr;

                nPixMapFlags = GetPixelsState( hPixMap );

                if ( nPixMapFlags == noErr )
                {
                    if ( LockPixels( hPixMap ) )
                    {
                        const short   nPixMapRowBytes    = GetNewPixMapRowBytes( nPixMapRowOffset );
                        const short   nPixMapColorDepth  = GetNewPixMapColorDepth( nPixMapBits );
                        const short   nPixMapCmpSize     = GetNewPixMapCmpSize( nPixMapBitDepth );
                        const short   nPixMapCmpCount    = GetNewPixMapCmpCount( nPixMapBitDepth );
                        const short   nPixMapPixelType   = GetNewPixMapPixelType( nPixMapBitDepth );
                        const OSType  aPixMapPixelFormat = GetNewPixMapPixelFormat( );
                        Rect          aPixMapBoundsRect;

                        GetNewPixMapBoudsRect( nPixMapWidth, nPixMapHeight, &aPixMapBoundsRect );

                        (**hPixMap).baseAddr    = pPixMapData;         // Pointer to pixels
                        (**hPixMap).rowBytes    = nPixMapRowBytes;     // Offset to next line
                        (**hPixMap).bounds      = aPixMapBoundsRect;   // Bounding bitmap rectangle
                        (**hPixMap).pmVersion   = 0;                   // PixMap version number
                        (**hPixMap).packType    = 0;                   // Defines packing format
                        (**hPixMap).packSize    = 0;                   // Length of pixel data
                        (**hPixMap).hRes        = 0x00480000;          // Horizontal resolution (ppi)
                        (**hPixMap).vRes        = 0x00480000;          // Vertical resolution (ppi)
                        (**hPixMap).pixelType   = nPixMapPixelType;    // Defines pixel type
                        (**hPixMap).pixelSize   = nPixMapBitDepth;     // Number of bits in a pixel
                        (**hPixMap).cmpCount    = nPixMapCmpCount;     // Number of components in a pixel
                        (**hPixMap).cmpSize     = nPixMapCmpSize;      // Number of bits per component
                        (**hPixMap).pixelFormat = aPixMapPixelFormat;  // Four character code representation
                        (**hPixMap).pmExt       = NULL;                // Handle to PixMap extension

                        // Get the color table based on the desired screen depth

                        (**hPixMap).pmTable = GetNewPixMapCTab( nPixMapBitDepth,
                                                                nPixMapColorDepth,
                                                                nPixMapCmpSize,
                                                                rBitmapPalette
                                                              );

                        SetPixelsState( hPixMap, nPixMapFlags );
                    } // if
                } // if
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

// ==================================================================

// ==================================================================

static PixMapHandle MallocPixMap ( const Size           &rPixMapSize,
                                   const USHORT          nPixMapBits,
                                   const BitmapPalette  &rBitmapPalette,
                                   const SalGraphics    *rSalGraphics
                                 )
{
    PixMapHandle hNewPixMap = GetPortPixMap( rSalGraphics->maGraphicsData.mpCGrafPort );

    if ( hNewPixMap == NULL )
    {
        hNewPixMap = GetNewPixMap( rPixMapSize,
                                   nPixMapBits,
                                   rBitmapPalette,
                                   rSalGraphics
                                 );
    } // else

    return hNewPixMap;
} // MallocPixMap

// ==================================================================

// ==================================================================

SalBitmap::SalBitmap() :
        mpVirDev( 0 ),
        mnBitCount( 0 )
{
}

// ------------------------------------------------------------------

SalBitmap::~SalBitmap()
{
    Destroy();
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const Size&           rSize,
                        USHORT                nBitCount,
                        const BitmapPalette&  rBitmapPalette
                      )
{
    ImplSVData  *pSVData           = ImplGetSVData();
    BOOL         bSalBitmapCreated = FALSE;

    if ( pSVData != NULL )
    {
        long nHeight = rSize.Height();
        long nWidth  = rSize.Width();

        Destroy();

        if ( ( nHeight > 0 ) && ( nWidth > 0 ) )
        {
            // Create a SalVirtualDevice

            mpVirDev
                = pSVData->mpDefInst->CreateVirtualDevice( NULL,
                                                           nWidth,
                                                           nHeight,
                                                           nBitCount
                                                         );

            if ( mpVirDev != NULL )
            {
                // Get the SalGraphics which contains the
                // GWorld we will draw to

                SalGraphics *pGraphics = GetGraphics();

                if (    ( pGraphics                             != NULL )
                     && ( pGraphics->maGraphicsData.mpCGrafPort != NULL )
                   )
                {
                    mhPixMap = MallocPixMap( rSize,
                                             nBitCount,
                                             rBitmapPalette,
                                             pGraphics
                                           );

                    if ( ( mhPixMap != NULL ) && ( *mhPixMap != NULL ) )
                    {
                        mnBitCount = GetPixDepth( mhPixMap);
                        maSize     = rSize;

                        bSalBitmapCreated = TRUE;
                    } // if
                } // if

                // Release the SalGraphics so that others can get a
                // handle to it in future GetGraphics() calls

                ReleaseGraphics( pGraphics );
            } // if
        } // if
    } // if

    return bSalBitmapCreated;
} // SalBitmap::Create

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp )
{
    return Create( rSalBmp, rSalBmp.GetBitCount() );
} // SalBitmap::Create

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp,
                        SalGraphics*     pGraphics
                      )
{
    USHORT nBitCount = 0;

    if (    ( pGraphics                             != NULL )
         && ( pGraphics->maGraphicsData.mpCGrafPort != NULL )
       )
    {
        mhPixMap = GetPortPixMap( pGraphics->maGraphicsData.mpCGrafPort );

        if ( mhPixMap != NULL )
        {
            nBitCount = GetPixDepth( mhPixMap );
        } // if
    } // if

    if ( nBitCount == 0 )
    {
        nBitCount = rSalBmp.GetBitCount();
    } // if

    return Create( rSalBmp, nBitCount );
} // SalBitmap::Create

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
{
    BOOL bSalBitmapCreated = FALSE;

    if ( Create( rSalBmp.maSize, nNewBitCount, BitmapPalette() ) )
    {
        // Copy pixels from rSalBmp.mpVirDev to mpVirDev

        SalGraphics *pDstGraphics = GetGraphics();

        if ( pDstGraphics != NULL )
        {
            SalGraphics *pSrcGraphics = rSalBmp.GetGraphics();

            if (    ( pSrcGraphics                             != NULL )
                 && ( pSrcGraphics->maGraphicsData.mpCGrafPort != NULL )
               )
            {
                long        nWidth  = rSalBmp.GetSize().Width();
                long        nHeight = rSalBmp.GetSize().Height();
                SalTwoRect  aTwoRect;

                // Get size of graphics to copy from

                aTwoRect.mnSrcX  = 0;
                aTwoRect.mnDestX = 0;
                aTwoRect.mnSrcY  = 0;
                aTwoRect.mnDestY = 0;

                aTwoRect.mnSrcWidth   = nWidth;
                aTwoRect.mnDestWidth  = nWidth;
                aTwoRect.mnSrcHeight  = nHeight;
                aTwoRect.mnDestHeight = nHeight;

                // Copy bits from source graphics

                pDstGraphics->CopyBits( &aTwoRect, pSrcGraphics );

                rSalBmp.ReleaseGraphics( pSrcGraphics );

                bSalBitmapCreated = TRUE;
            } // if

            ReleaseGraphics( pDstGraphics );
        } // if
    } // if

    return bSalBitmapCreated;
} // SalBitmap::Create

// ------------------------------------------------------------------

void SalBitmap::Destroy()
{
    ImplSVData *pSVData = ImplGetSVData();

    if ( pSVData != NULL )
    {
        if ( mpVirDev != NULL )
        {
            pSVData->mpDefInst->DestroyVirtualDevice( mpVirDev );
        } // if

        mhPixMap   = NULL;
        maSize     = Size();
        mnBitCount = 0;
    } // if
} // SalBitmap::Destroy

// ------------------------------------------------------------------

BitmapBuffer* SalBitmap::AcquireBuffer( BOOL bReadOnly )
{
    BitmapBuffer *pBuffer = NULL;

    if ( mpVirDev != NULL )
    {
        // Get the SalGraphics which contains the GWorld we will draw to

        SalGraphics *pGraphics = GetGraphics();

        if (    ( pGraphics                             != NULL )
             && ( pGraphics->maGraphicsData.mpCGrafPort != NULL )
           )
        {
            if ( mhPixMap != NULL )
            {
                // Lock the GWorld so that the calling functions
                // can write to this buffer

                pGraphics->maGraphicsData.mnOSStatus
                    = LockPortBits( pGraphics->maGraphicsData.mpCGrafPort );

                if ( pGraphics->maGraphicsData.mnOSStatus == noErr )
                {
                    pBuffer = new BitmapBuffer();

                    if ( pBuffer != NULL )
                    {
                        pBuffer->mnFormat   = BMP_FORMAT_TOP_DOWN;
                        pBuffer->mnBitCount = GetPixDepth( mhPixMap);

                        switch ( pBuffer->mnBitCount )
                        {
                            case kBlackAndWhite:
                                pBuffer->mnFormat |= BMP_FORMAT_1BIT_MSB_PAL;
                                break;
                            case kFourBitColor:
                                pBuffer->mnFormat |= BMP_FORMAT_4BIT_MSN_PAL;
                                break;
                            case kEightBitColor:
                                pBuffer->mnFormat |= BMP_FORMAT_8BIT_PAL;
                                break;
                            case kThousandsColor:
                                pBuffer->mnFormat    |= BMP_FORMAT_16BIT_TC_MASK;
                                pBuffer->maColorMask  = ColorMask( 0x7b00, 0x03e0, 0x001f);
                                break;
                            case kTrueColor:
                                pBuffer->mnFormat |= BMP_FORMAT_32BIT_TC_ARGB;
                                break;
                            default:
                                break;
                        } // switch

                        if ( BMP_SCANLINE_FORMAT( pBuffer->mnFormat ) )
                        {
                            GWorldFlags  nGWorldFlags = noErr;

                            pBuffer->mnWidth  = mpVirDev->maVirDevData.mnWidth;
                            pBuffer->mnHeight = mpVirDev->maVirDevData.mnHeight;

                            nGWorldFlags = GetPixelsState( mhPixMap );

                            if ( nGWorldFlags == noErr )
                            {
                                LockPixels( mhPixMap );

                                pBuffer->mnScanlineSize = GetPixRowBytes( mhPixMap );
                                pBuffer->mpBits         = (BYTE *)GetPixBaseAddr( mhPixMap );

                                // If the pixel depth is <= 8, we need to map QD's
                                // internal color table to the platform independent
                                // BitmapPalette color table

                                if ( pBuffer->mnBitCount <= 8 )
                                {
                                    CTabHandle hCTab = (**mhPixMap).pmTable;

                                    if ( ( hCTab != NULL ) && ( *hCTab != NULL ) )
                                    {
                                        SInt8 nCTabFlags  = noErr;

                                        nCTabFlags = HGetState( (Handle)hCTab );

                                        if ( nCTabFlags == noErr )
                                        {
                                            USHORT          nCTabCount     = 0;
                                            USHORT          nCTabIndex     = 0;
                                            BitmapPalette  &rBitmapPalette = pBuffer->maPalette;

                                            HLock( (Handle)hCTab );

                                            // Map each color in the QuickDraw color
                                            // table to a BitmapColor

                                            nCTabCount = (**hCTab).ctSize + 1;

                                            rBitmapPalette.SetEntryCount( nCTabCount );

                                            for ( nCTabIndex = 0;
                                                  nCTabIndex < nCTabCount;
                                                  nCTabIndex++
                                                )
                                            {
                                                BitmapColor &rBitmapColor = rBitmapPalette[nCTabIndex];
                                                ColorSpec    aColorSpec   = (**hCTab).ctTable[nCTabIndex];

                                                rBitmapColor.SetRed   ( (BYTE)( aColorSpec.rgb.red   >> 8 ) );
                                                rBitmapColor.SetGreen ( (BYTE)( aColorSpec.rgb.green >> 8 ) );
                                                rBitmapColor.SetBlue  ( (BYTE)( aColorSpec.rgb.blue  >> 8 ) );
                                            } // for

                                            HSetState( (Handle)hCTab, nCTabFlags );
                                        } // if
                                    } // if
                                } // if

                                SetPixelsState( mhPixMap, nGWorldFlags );
                            } // if
                        } // if
                        else
                        {
                            UnlockPortBits( pGraphics->maGraphicsData.mpCGrafPort );

                            ReleaseGraphics( pGraphics );

                            delete pBuffer;
                        } // else
                    } // if

                    if ( pGraphics != NULL )
                    {
                        UnlockPortBits( pGraphics->maGraphicsData.mpCGrafPort );
                    } // if
                } // if
            } // if

            // Release the SalGraphics so that others can get a
            // handle to it in future GetGraphics() calls

            if ( pGraphics != NULL )
            {
                ReleaseGraphics( pGraphics );
            } // if
        } // if
    } // if

    return pBuffer;
} // SalBitmap::AcquireBuffer

// ------------------------------------------------------------------

void SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
    if ( mpVirDev != NULL )
    {
        // Get the SalGraphics which contains the GWorld
        // we used as the buffer

        SalGraphics *pGraphics = GetGraphics();

        if (    ( pGraphics                             != NULL )
             && ( pGraphics->maGraphicsData.mpCGrafPort != NULL )
           )
        {
            // Release the SalGraphics so that others can get a
            // handle to it in future GetGraphics() calls

            ReleaseGraphics( pGraphics );

            pGraphics = NULL;
        } // if
    } // if

    if ( pBuffer != NULL )
    {
        delete pBuffer;

        pBuffer = NULL;
    } // if
} // SalBitmap::ReleaseBuffer

// ------------------------------------------------------------------

SalGraphics* SalBitmap::GetGraphics()
{
    if ( mpVirDev != NULL )
    {
        return mpVirDev->GetGraphics();
    } // if
    else
    {
        return NULL;
    } // else
} // SalBitmap::GetGraphics

// ------------------------------------------------------------------

void SalBitmap::ReleaseGraphics( SalGraphics* pGraphics )
{
    if ( mpVirDev != NULL )
    {
        mpVirDev->ReleaseGraphics( pGraphics );
    } // if
} // SalBitmap::ReleaseGraphics

// ==================================================================


