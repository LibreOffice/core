/*************************************************************************
 *
 *  $RCSfile: salcolorutils.cxx,v $
 *
 *  $Revision: 1.3 $
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

#define _SV_SALCOLORUTILS_CXX

#ifndef _SV_SALCOLORUTILS_HXX
    #include <salcolorutils.hxx>
#endif

// =======================================================================

// =======================================================================

static unsigned long  Compress32BitRGBColor ( const RGBColor *pRGBColor )
{
    unsigned long nCTableIndex = 0;

    nCTableIndex  = ( pRGBColor->red   >> kOneByte ) << kTwoBytes;
    nCTableIndex |= ( pRGBColor->green >> kOneByte ) << kOneByte;
    nCTableIndex |=   pRGBColor->blue  >> kOneByte;

    return nCTableIndex;
} // Compress32BitRGBColor

// -----------------------------------------------------------------------

static unsigned long  Compress16BitRGBColor ( const RGBColor *pRGBColor )
{
    unsigned long nCTableIndex = 0;

    nCTableIndex  = ( pRGBColor->red   >> kElevenBits ) << kTenBits;
    nCTableIndex |= ( pRGBColor->green >> kElevenBits ) << kFiveBits;
    nCTableIndex |=   pRGBColor->blue  >> kElevenBits;

    return nCTableIndex;
} // Compress16BitRGBColor

// -----------------------------------------------------------------------

static void CompressRGBColor ( const PixMapPtr   pPixMap,
                               const RGBColor   *pRGBColor,
                               SalColor         *rSalColor
                             )
{
    if ( pPixMap->pixelSize == kTrueColor )
    {
        *rSalColor = Compress32BitRGBColor( pRGBColor );
    } // if
    else
    {
        *rSalColor = Compress16BitRGBColor( pRGBColor );
    } // else
} // CompressRGBColor

// -----------------------------------------------------------------------
//
// Here we shall utilize an approximation to Euclidian  norm.  Where as
// in a standard Euclidian norm one sums up the square of the RGB color
// distances before proceeding to take the square root, in our case, we
// only measure the RGB color difference before summing up the totals.
// This way of measuring distance is also called the "Manhattan distance."
// Manhattan distance takes advantage of the fact that the sum of the
// lengths of the three components of a 3D vector is a rough approxima-
// tion of the vector's length.
//
// -----------------------------------------------------------------------

static unsigned long  RGBDistance ( const RGBColor  *pRGBColor1,
                                    const RGBColor  *pRGBColor2
                                  )
{
    unsigned long  nRGBDist = 0;
    LRectCoor      aVec[2];

    // Find |r'-r|, |g'-g|, and |b'-b| from (r,g,b) and (r',g',b')
    // coordinates; where red lies on the x-axis, green lies on the
    // y-axis, and blue lies on the z-axis of the RGB color cube.

    aVec[0].x = (long)pRGBColor1->red;
    aVec[0].y = (long)pRGBColor1->green;
    aVec[0].z = (long)pRGBColor1->blue;

    aVec[1].x = (long)pRGBColor2->red;
    aVec[1].y = (long)pRGBColor2->green;
    aVec[1].z = (long)pRGBColor2->blue;

    // Now Compute || (r',g',b') - (r,g,b) ||

    nRGBDist = Euclidian2Norm( aVec );

    return nRGBDist;
} // RGBDistance

// -----------------------------------------------------------------------

static inline BOOL QDColorsMatch ( const RGBColor  *pRGBColor1,
                                   const RGBColor  *pRGBColor2
                                 )
{
    if (    ( pRGBColor2->red   == pRGBColor1->red   )
         && ( pRGBColor2->green == pRGBColor1->green )
         && ( pRGBColor2->blue  == pRGBColor1->blue  )
       )
    {
        return TRUE;
    } // if
    else
    {
        return FALSE;
    } // else
} // QDColorsMatch

// -----------------------------------------------------------------------

static void GetBestSalColor ( const CTabPtr    pCTable,
                              const RGBColor  *pRGBColor,
                              unsigned long   *pBestSalColor
                            )
{
    unsigned short   nCTableSize     = 0;
    unsigned long    nCTableIndex    = 0;
    unsigned long    nRGBNewDistance = 0;
    unsigned long    nRGBMinDistance = UID_MAX;
    RGBColor        *pRGBNextColor   = NULL;
    BOOL             bRGBColorsMatch = FALSE;

    *pBestSalColor = 0;

    nCTableSize = pCTable->ctSize;

    while ( ( nCTableIndex < nCTableSize ) && ( bRGBColorsMatch == FALSE ) )
    {
        pRGBNextColor = &pCTable->ctTable[nCTableIndex].rgb;

        bRGBColorsMatch = QDColorsMatch( pRGBColor, pRGBNextColor );

        if ( bRGBColorsMatch == TRUE )
        {
            *pBestSalColor = nCTableIndex;
        } // if
        else
        {
            nRGBNewDistance = RGBDistance( pRGBColor, pRGBNextColor );

            if ( nRGBNewDistance < nRGBMinDistance )
            {
                 nRGBMinDistance = nRGBNewDistance;
                *pBestSalColor   = nCTableIndex;
            } // if

            nCTableIndex++;
        } //else
    } // while
} // GetBestSalColor

// -----------------------------------------------------------------------

static void GetCTableIndex ( const RGBColor   *pRGBColor,
                             SalColor         *rSalColor
                           )
{
    GDPtr pGDevice = NULL;

    pGDevice = *GetGDevice();

    if ( pGDevice != NULL )
    {
        PixMapPtr  pPixMap = *pGDevice->gdPMap;

        if ( pPixMap != NULL )
        {
            CTabPtr  pCTable = *pPixMap->pmTable;

            if ( pCTable != NULL )
            {
                ITabPtr  pITable = *pGDevice->gdITable;

                if ( pITable != NULL )
                {
                    // Is the inverse color table up-to-date?

                    if ( pITable->iTabSeed != pCTable->ctSeed )
                    {
                        // Update our inverse color table

                        MakeITable( pPixMap->pmTable,
                                    pGDevice->gdITable,
                                    pGDevice->gdResPref
                                  );

                        pGDevice = *GetGDevice();
                        pITable  = *pGDevice->gdITable;
                        pPixMap  = *pGDevice->gdPMap;
                        pCTable  = *pPixMap->pmTable;
                    } // if
                } // if

                // Now we proceed to find the closest match in our
                // color table.

                if ( pCTable != NULL )
                {
                    GetBestSalColor( pCTable, pRGBColor, rSalColor );

                    if ( *rSalColor > pCTable->ctSize )
                    {
                        // Return the index for white color in case
                        // of an error

                        *rSalColor = 255;
                    } // if
                } // if
            } // if
        } // if
    } // if
} // GetCTableIndex

// -----------------------------------------------------------------------
//
// Convert RGB color to Sal color.
//
// -----------------------------------------------------------------------

SalColor RGBColor2SALColor ( const RGBColor *pRGBColor )
{
    GDPtr      pGDevice  = NULL;
    SalColor   nSalColor = 0;

    pGDevice = *GetGDevice( );

    if ( pGDevice != NULL )
    {
        PixMapPtr  pPixMap = NULL;

        pPixMap = *(*pGDevice).gdPMap;

        if ( pPixMap != NULL )
        {
            if ( pGDevice->gdType == directType )
            {
                CompressRGBColor( pPixMap, pRGBColor, &nSalColor );
            } // if
            else
            {
                GetCTableIndex( pRGBColor, &nSalColor );
            } // else
        } // if
    } // if

    return  nSalColor;
} // RGBColor2SALColor

// =======================================================================

// =======================================================================

SalColor RGB8BitColor2SALColor ( const RGBColor *pRGBColor )
{
    GDPtr      pGDevice  = NULL;
    SalColor   nSalColor = 0;

    pGDevice = *GetGDevice( );

    if ( pGDevice != NULL )
    {
        PixMapPtr  pPixMap = *(*pGDevice).gdPMap;

        if ( pPixMap != NULL )
        {
            if ( pGDevice->gdType != directType )
            {
                GetCTableIndex( pRGBColor, &nSalColor );
            } // else
        } // if
    } // if

    return  nSalColor;
} // RGB8BitColor2SALColor

// -----------------------------------------------------------------------

SalColor RGB16BitColor2SALColor ( const RGBColor *pRGBColor )
{
    GDPtr      pGDevice  = NULL;
    SalColor   nSalColor = 0;

    pGDevice = *GetGDevice( );

    if ( pGDevice != NULL )
    {
        PixMapPtr  pPixMap = *(*pGDevice).gdPMap;

        if ( pPixMap != NULL )
        {
            if ( pGDevice->gdType == directType )
            {
                if ( pPixMap->pixelSize == kThousandsColor )
                {
                    nSalColor = Compress16BitRGBColor( pRGBColor );
                } // if
            } // if
        } // if
    } // if

    return  nSalColor;
} // RGB16BitColor2SALColor

// -----------------------------------------------------------------------

SalColor RGB32BitColor2SALColor ( const RGBColor *pRGBColor )
{
    GDPtr      pGDevice  = NULL;
    SalColor   nSalColor = 0;

    pGDevice = *GetGDevice( );

    if ( pGDevice != NULL )
    {
        PixMapPtr  pPixMap = *(*pGDevice).gdPMap;

        if ( pPixMap != NULL )
        {
            if ( pGDevice->gdType == directType )
            {
                if ( pPixMap->pixelSize == kTrueColor )
                {
                    nSalColor = Compress32BitRGBColor( pRGBColor );
                } // if
            } // if
        } // if
    } // if

    return  nSalColor;
} // RGB32BitColor2SALColor

// =======================================================================

// =======================================================================

static unsigned short SalColorTo32BitDeviceColor ( SalColor  *rSalColor )
{
    unsigned short  nDirectColor = 0;
    unsigned short  nUpperByte   = 0;
    unsigned short  nLowerByte   = 0;

     nLowerByte     = *rSalColor   & kByteMask;
     nUpperByte     =  nLowerByte << kOneByte;
     nDirectColor   =  nUpperByte  | nLowerByte;
    *rSalColor    >>=  kOneByte;

    return  nDirectColor;
} // SalColorTo32BitDeviceColor

// -----------------------------------------------------------------------

static void SalColor2DirectColor ( const SalColor   nSalColor,
                                   RGBColor        *rRGBColor
                                 )
{
    SalColor  nIndexedColor = nSalColor;

    rRGBColor->blue  = SalColorTo32BitDeviceColor( &nIndexedColor );
    rRGBColor->green = SalColorTo32BitDeviceColor( &nIndexedColor );
    rRGBColor->red   = SalColorTo32BitDeviceColor( &nIndexedColor );

} // SalColor2DirectColor

// -----------------------------------------------------------------------

static unsigned short SalColorTo16BitDeviceColor ( SalColor  *rSalColor )
{
    unsigned short  nColor16Bit  = 0;
    unsigned short  nUpper5Bits  = 0;
    unsigned short  nUpper10Bits = 0;
    unsigned short  nLower5Bits  = 0;

     nUpper5Bits    = *rSalColor    << kElevenBits;
     nUpper10Bits   =  nUpper5Bits   | ( nUpper5Bits >> kFiveBits );
     nLower5Bits    =  nUpper10Bits >> kTenBits;
     nColor16Bit    =  nUpper10Bits  | nLower5Bits;
    *rSalColor    >>=  kFiveBits;

    return  nColor16Bit;
}

// -----------------------------------------------------------------------

static void  SalColor2ThousandsColor ( const SalColor   nSalColor,
                                       RGBColor        *rRGBColor
                                     )
{
    SalColor  nIndexedColor = nSalColor;

    rRGBColor->blue  = SalColorTo16BitDeviceColor ( &nIndexedColor );
    rRGBColor->green = SalColorTo16BitDeviceColor ( &nIndexedColor );
    rRGBColor->red   = SalColorTo16BitDeviceColor ( &nIndexedColor );
} // SalColor2ThousandsColor

// -----------------------------------------------------------------------

static void SalColor2EightBitColor ( const SalColor   nSalColor,
                                     RGBColor        *rRGBColor
                                   )
{
    GDPtr pGDevice = NULL;

    pGDevice = *GetGDevice();

    if ( pGDevice != NULL )
    {
        CTabPtr    pCTable = NULL;
        PixMapPtr  pPixMap = NULL;

        pPixMap = *(*pGDevice).gdPMap;

        if ( pPixMap != NULL )
        {
            pCTable = *(*pPixMap).pmTable;

            if ( pCTable != NULL )
            {
                if ( nSalColor <= pCTable->ctSize )
                {
                    RGBColor  aRGBColor;

                    aRGBColor        = pCTable->ctTable[nSalColor].rgb;
                    rRGBColor->red   = aRGBColor.red;
                    rRGBColor->green = aRGBColor.green;
                    rRGBColor->blue  = aRGBColor.blue;
                } // if
            } // if
        } // if
    } // if
} // SalColor2EightBitColor

// -----------------------------------------------------------------------
//
// Here we will convert SAL color to either 8-bit or 32-bit color.
// For 16-bit color we shall let Mac OS compute the nearest color
// from that of 32-bit color using the Euclidean 2-norm in RGB color
// space.
//
// -----------------------------------------------------------------------

RGBColor SALColor2RGBColor ( const SalColor nSalColor )
{
    GDPtr     pGDevice = NULL;
    RGBColor  aRGBColor;

    memset( &aRGBColor, 0, sizeof(RGBColor) );

    pGDevice = *GetGDevice();

    if ( pGDevice != NULL )
    {
        if ( pGDevice->gdType == directType )
        {
            SalColor2DirectColor( nSalColor, &aRGBColor );
        } // if
        else
        {
            SalColor2EightBitColor( nSalColor, &aRGBColor );
        } // else
    } // if

    return aRGBColor;
} // SALColor2RGBColor

// =======================================================================

// =======================================================================

RGBColor SALColor2RGB32bitColor ( const SalColor nSalColor )
{
    RGBColor aRGBColor;

    SalColor2DirectColor ( nSalColor, &aRGBColor );

    return aRGBColor;
} // SALColor2RGB32bitColor

// -----------------------------------------------------------------------

RGBColor SALColor2RGB16bitColor ( const SalColor nSalColor )
{
    RGBColor aRGBColor;

    SalColor2ThousandsColor ( nSalColor, &aRGBColor );

    return aRGBColor;
} // SALColor2RGB16bitColor

// -----------------------------------------------------------------------

RGBColor SALColor2RGB8bitColor ( const SalColor nSalColor )
{
    RGBColor aRGBColor;

    SalColor2EightBitColor ( nSalColor, &aRGBColor );

    return aRGBColor;
} // SALColor2RGB8bitColor

// =======================================================================

// =======================================================================

SalColor GetROPSalColor( SalROPColor nROPColor )
{
    SalColor  nSalColor = 0;

    if ( nROPColor == SAL_ROP_0 )
    {
        RGBColor  aBlackColor;

        aBlackColor.red   = 0x0000;
        aBlackColor.green = 0x0000;
        aBlackColor.blue  = 0x0000;

        nSalColor = RGBColor2SALColor ( &aBlackColor );
    } // if
    else
    {
        RGBColor  aWhiteColor;

        aWhiteColor.red   = 0xffff;
        aWhiteColor.green = 0xffff;
        aWhiteColor.blue  = 0xffff;

        nSalColor = RGBColor2SALColor ( &aWhiteColor );
    } // else

    return nSalColor;
} // GetROPSalColor

// =======================================================================

// =======================================================================

short GetMinColorCount( const short          nPixMapColorDepth,
                        const BitmapPalette  &rBitmapPalette
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

// =======================================================================

// =======================================================================

RGBColor BitmapColor2RGBColor ( const BitmapColor &rBitmapColor )
{
    const USHORT  nBitmapRedColor   = rBitmapColor.GetRed();
    const USHORT  nBitmapGreenColor = rBitmapColor.GetGreen();
    const USHORT  nBitmapBlueColor  = rBitmapColor.GetBlue();
    RGBColor      aRGBColor;

    aRGBColor.red   = (short)(( nBitmapRedColor   << kOneByte ) | nBitmapRedColor   );
    aRGBColor.green = (short)(( nBitmapGreenColor << kOneByte ) | nBitmapGreenColor );
    aRGBColor.blue  = (short)(( nBitmapBlueColor  << kOneByte ) | nBitmapBlueColor  );

    return aRGBColor;
} // BitmapColor2RGBColor

// ------------------------------------------------------------------

void RGBColor2BitmapColor ( const RGBColor  *rRGBColor,
                            BitmapColor     &rBitmapColor
                          )
{
    if ( rRGBColor != NULL )
    {
        const BYTE  nRedColor   = (BYTE)( rRGBColor->red   >> kOneByte );
        const BYTE  nGreenColor = (BYTE)( rRGBColor->green >> kOneByte );
        const BYTE  nBlueColor  = (BYTE)( rRGBColor->blue  >> kOneByte );

        rBitmapColor.SetRed   ( nRedColor   );
        rBitmapColor.SetGreen ( nGreenColor );
        rBitmapColor.SetBlue  ( nBlueColor  );
    } // if
} // RGBColor2BitmapColor

// =======================================================================

// =======================================================================

void SetBlackForeColor ( )
{
    RGBColor aBlackForeColor;

    // Black color

    aBlackForeColor.red   = 0x0000;
    aBlackForeColor.green = 0x0000;
    aBlackForeColor.blue  = 0x0000;

    // Set foreground color to black

    RGBForeColor( &aBlackForeColor );
} // SetBlackForeColor

// -----------------------------------------------------------------------

void SetWhiteBackColor ( )
{
    RGBColor aWhiteBackColor;

    // White color

    aWhiteBackColor.red   = 0xffff;
    aWhiteBackColor.green = 0xffff;
    aWhiteBackColor.blue  = 0xffff;

    // Set background color to white

    RGBBackColor( &aWhiteBackColor );
} // SetWhiteBackColor

// =======================================================================

// =======================================================================

RGBColor GetBlackColor ( )
{
    RGBColor aBlackColor;

    aBlackColor.red   = 0x0000;
    aBlackColor.green = 0x0000;
    aBlackColor.blue  = 0x0000;

    return aBlackColor;
} // GetBlackColor

// -----------------------------------------------------------------------

RGBColor GetWhiteColor ( )
{
    RGBColor aWhiteColor;

    aWhiteColor.red   = 0xffff;
    aWhiteColor.green = 0xffff;
    aWhiteColor.blue  = 0xffff;

    return aWhiteColor;
} // GetWhiteColor

// =======================================================================

// =======================================================================

CTabHandle CopyGDeviceCTab ( )
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

CTabHandle GetCTabFromStdCLUT ( const short nBitDepth )
{
    CTabHandle  hCTab   = NULL;
    short       nCLUTID = 0;

    nCLUTID = nBitDepth + 64;

    hCTab = GetCTable( nCLUTID );

    return hCTab;
} // GetCTabFromStdCLUT

// =======================================================================

// =======================================================================

CTabHandle CopyCTabIndexed ( CTabHandle hCTab )
{
    CTabHandle  hCTabCopy = NULL;

    if ( ( hCTab != NULL ) && ( *hCTab != NULL ) )
    {
        SInt8 nhCTabFlags = noErr;

        nhCTabFlags = HGetState( (Handle)hCTabCopy );

        if ( nhCTabFlags == noErr )
        {
            long  nCTabSize       = 0;
            long  nCTabHandleSize = 0;

            HLock( (Handle)hCTab );

                nCTabSize = (**hCTab).ctSize + 1;

                nCTabHandleSize =   (   nCTabSize
                                      * sizeof( ColorSpec )
                                    )
                                  + (   sizeof( ColorTable )
                                      - sizeof( CSpecArray )
                                    );

                hCTabCopy = (CTabHandle)NewHandleClear( nCTabHandleSize );

                if ( ( hCTabCopy != NULL ) && ( *hCTabCopy != NULL ) )
                {
                    SInt8  nCTabCopyFlags = noErr;

                    nCTabCopyFlags = HGetState( (Handle)hCTabCopy );

                    if ( nCTabCopyFlags == noErr )
                    {
                        unsigned long  nPortPixMapCTabIndex;

                        HLock( (Handle)hCTabCopy );

                            (**hCTabCopy).ctSize  = (**hCTab).ctSize;
                            (**hCTabCopy).ctFlags = (**hCTab).ctFlags;
                            (**hCTabCopy).ctSeed  = GetCTSeed();

                            for ( nPortPixMapCTabIndex = 0;
                                  nPortPixMapCTabIndex < nCTabSize;
                                  nPortPixMapCTabIndex++
                                )
                            {
                                (**hCTabCopy).ctTable[nPortPixMapCTabIndex]
                                    = (**hCTab).ctTable[nPortPixMapCTabIndex];
                            } // for

                        HSetState( (Handle)hCTabCopy, nCTabCopyFlags );
                    } // if
                } // if

            HSetState( (Handle)hCTab, nhCTabFlags );
        } // if
    } // if
    else
    {
        hCTabCopy = GetCTabFromStdCLUT( kEightBitColor );

        if ( hCTabCopy == NULL )
        {
            hCTabCopy = CopyGDeviceCTab( );
        } // if
    } // else

    return hCTabCopy;
} // CopyCTabIndexed

// ------------------------------------------------------------------

CTabHandle CopyCTabRGBDirect ( CTabHandle hCTab )
{
    CTabHandle  hCTabCopy = NULL;

    if ( ( hCTab != NULL ) && ( *hCTab != NULL ) )
    {
        SInt8 nhCTabFlags = noErr;

        nhCTabFlags = HGetState( (Handle)hCTabCopy );

        if ( nhCTabFlags == noErr )
        {
            HLock( (Handle)hCTab );

                hCTabCopy = (CTabHandle)NewHandleClear(   sizeof( ColorTable )
                                                        - sizeof( CSpecArray )
                                                      );

                if ( ( hCTabCopy != NULL ) && ( *hCTabCopy != NULL ) )
                {
                    SInt8  nCTabCopyFlags = noErr;

                    nCTabCopyFlags = HGetState( (Handle)hCTabCopy );

                    if ( nCTabCopyFlags == noErr )
                    {
                        HLock( (Handle)hCTabCopy );

                            (**hCTabCopy).ctSeed  = (**hCTab).ctSeed;
                            (**hCTabCopy).ctFlags = (**hCTab).ctFlags;
                            (**hCTabCopy).ctSize  = (**hCTab).ctSize;

                        HSetState( (Handle)hCTabCopy, nCTabCopyFlags );
                    } // if
                } // if

            HSetState( (Handle)hCTab, nhCTabFlags );
        } // if
    } // if

    return hCTabCopy;
} // CopyCTabRGBDirect

// =======================================================================

// =======================================================================

CTabHandle CopyPixMapCTab ( PixMapHandle hPixMap )
{
    CTabHandle  hCTabCopy       = NULL;
    short       nPixMapBitDepth = GetPixDepth( hPixMap );

    if ( nPixMapBitDepth <= kEightBitColor )
    {
        hCTabCopy = CopyCTabIndexed( (**hPixMap).pmTable );
    } // if
    else
    {
        hCTabCopy = CopyCTabRGBDirect( (**hPixMap).pmTable );
    } // else

    return  hCTabCopy;
} // CopyPixMapCTab

// =======================================================================

// =======================================================================

void SetBitmapBufferColorFormat ( const PixMapHandle   mhPixMap,
                                  BitmapBuffer        *rBuffer
                                )
{
    const short nPixMapCmpCount = (**mhPixMap).cmpCount;
    const short nPixMapCmpSize  = (**mhPixMap).cmpSize;

    rBuffer->mnFormat = BMP_FORMAT_TOP_DOWN;

    if( nPixMapCmpCount == 1 )
    {
        // Indexed color mode

        switch ( nPixMapCmpSize )
        {
            case ( kPixMapCmpSizeOneBit ):
                rBuffer->mnFormat |= BMP_FORMAT_1BIT_MSB_PAL;
                break;
            case ( kPixMapCmpSizeFourBits ):
                rBuffer->mnFormat |= BMP_FORMAT_4BIT_MSN_PAL;
                break;
            case ( kPixMapCmpSizeEightBits ):
                rBuffer->mnFormat |= BMP_FORMAT_8BIT_PAL;
                break;
            default:
                break;
        } // switch
    } // if
    else
    {
        // Direct color mode

        switch ( nPixMapCmpSize )
        {
            case ( kPixMapCmpSizeFiveBits ):
                rBuffer->mnFormat    |= BMP_FORMAT_16BIT_TC_MASK;
                rBuffer->maColorMask  = ColorMask( k16BitRedColorMask,
                                                   k16BitGreenColorMask,
                                                   k16BitBlueColorMask
                                                 );
                break;
            case ( kPixMapCmpSizeEightBits ):
                rBuffer->mnFormat    |= BMP_FORMAT_32BIT_TC_ARGB;
                rBuffer->maColorMask  = ColorMask( k32BitRedColorMask,
                                                   k32BitGreenColorMask,
                                                   k32BitBlueColorMask
                                                 );
                break;
            default:
                break;
        } // switch;
    } //else
} // SetBitmapBufferColorFormat

// =======================================================================

// =======================================================================

