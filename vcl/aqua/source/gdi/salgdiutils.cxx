/*************************************************************************
 *
 *  $RCSfile: salgdiutils.cxx,v $
 *
 *  $Revision: 1.4 $
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

#define _SV_SALGDIUTILS_CXX

#ifndef _SV_SALGDIUTILS_HXX
    #include <salgdiutils.hxx>
#endif

// =======================================================================

// =======================================================================

short SelectCopyMode ( const SalGraphicsDataPtr pSalGraphicsData )
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

RgnHandle GetPolygonRgn ( const unsigned long   nPolyCount,
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

            for ( nPolyIndex = 0;
                  nPolyIndex < nPolyCount;
                  nPolyIndex++
                )
            {
                const SalPoint *pPtAry = ppPtAry[nPolyIndex];

                nPolyEdges = pPoints[nPolyIndex];

                // Begin region construction

                OpenRgn();

                    // Begin polygon construction

                    MoveTo( pPtAry[0].mnX,  pPtAry[0].mnY );

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

OSStatus GetGDeviceBitDepth ( unsigned short  *rGDeviceBitDepth )
{
    GDPtr     pGDevice  = NULL;
    OSStatus  nOSStatus = noErr;

    pGDevice = *GetGDevice ( );

    nOSStatus = QDErr();

    if ( ( pGDevice != NULL ) && ( nOSStatus == noErr ) )
    {
        *rGDeviceBitDepth = GetPixDepth( pGDevice->gdPMap );
    } // if

    return nOSStatus;
} // GetGDeviceBitDepth

// -----------------------------------------------------------------------

OSStatus GetGDeviceResolution ( long  *rGDeviceHRes,
                                long  *rGDeviceVRes
                              )
{
    GDPtr     pGDevice  = NULL;
    OSStatus  nOSStatus = noErr;

    pGDevice = *GetGDevice ( );

    nOSStatus = QDErr();

    if ( ( pGDevice != NULL ) && ( nOSStatus == noErr ) )
    {
        long  nGDeviceTop    = pGDevice->gdRect.top;
        long  nGDeviceLeft   = pGDevice->gdRect.left;
        long  nGDeviceBottom = pGDevice->gdRect.bottom;
        long  nGDeviceRight  = pGDevice->gdRect.right;

        // From the current GDevice get its horizontal resolution

        *rGDeviceHRes = abs( nGDeviceRight - nGDeviceLeft );

        // From the current GDevice get its vertical resolution

        *rGDeviceVRes = abs( nGDeviceBottom - nGDeviceTop );
    } // if

    return nOSStatus;
} // GetGDeviceResolution

// =======================================================================

// =======================================================================

BOOL LockGraphics ( SalGraphics *rSalGraphics )
{
    BOOL  bCGrafPortLocked = FALSE;

    if (    ( rSalGraphics                             != NULL )
         && ( rSalGraphics->maGraphicsData.mpCGrafPort != NULL )
       )
    {
        rSalGraphics->maGraphicsData.mnOSStatus
            = LockPortBits( rSalGraphics->maGraphicsData.mpCGrafPort );

        if ( rSalGraphics->maGraphicsData.mnOSStatus == noErr )
        {
            rSalGraphics->maGraphicsData.mhGWorldPixMap
                = GetPortPixMap( rSalGraphics->maGraphicsData.mpCGrafPort );

            rSalGraphics->maGraphicsData.mnOSStatus = QDErr();

            bCGrafPortLocked = TRUE;
        } // if
    } // if

    return  bCGrafPortLocked;
} // LockGraphics

// -----------------------------------------------------------------------

BOOL UnlockGraphics ( SalGraphics *rSalGraphics )
{
    BOOL  bCGrafPortUnlocked = FALSE;

    if (    ( rSalGraphics->maGraphicsData.mpCGrafPort != NULL  )
         && ( rSalGraphics->maGraphicsData.mnOSStatus  == noErr )
       )
    {
        rSalGraphics->maGraphicsData.mnOSStatus
            = UnlockPortBits( rSalGraphics->maGraphicsData.mpCGrafPort );

        bCGrafPortUnlocked = TRUE;
    } // if

    return bCGrafPortUnlocked;
} // UnlockGraphics

// =======================================================================

// =======================================================================

BOOL BeginGraphics ( SalGraphicsDataPtr rSalGraphicsData )
{
    BOOL bStartGraphics = FALSE;

    // Previous to entering this function, was there a QD error?

    if ( rSalGraphicsData->mnOSStatus == noErr )
    {
        // Get the graph port and lock focus on it

        if ( rSalGraphicsData->mbWindow == TRUE )
        {
            rSalGraphicsData->mpCGrafPort
                = VCLGraphics_LockFocusCGrafPort( rSalGraphicsData->mhDC );
        } // if

        if ( rSalGraphicsData->mpCGrafPort != NULL )
        {
            // Get the port's pen attributes

            GetPortPenPixPat( rSalGraphicsData->mpCGrafPort,
                                          rSalGraphicsData->mhPortPenPattern
                                        );

             GetPortPenSize(   rSalGraphicsData->mpCGrafPort,
                                         &(rSalGraphicsData->maPortPenSize)
                                       );

            GetPortPenLocation(   rSalGraphicsData->mpCGrafPort,
                                            &(rSalGraphicsData->maPortPenLocation)
                                          );

            rSalGraphicsData->mnPortPenMode
                = GetPortPenMode( rSalGraphicsData->mpCGrafPort );

            // Set the port pen mode to its new value

            SetPortPenMode( rSalGraphicsData->mpCGrafPort,
                            rSalGraphicsData->mnPenMode
                          );

            // Set to the current offscreen world for Mac OS X
            // only as everything is written to GWorld here

            SetGWorld( rSalGraphicsData->mpCGrafPort, NULL );

            // Was there a QD error when we set the GWorld?

            rSalGraphicsData->mnOSStatus = QDErr();

            if ( rSalGraphicsData->mnOSStatus == noErr )
            {
                // Get the PixMap associated with this particular CGrafPort

                rSalGraphicsData->mhGWorldPixMap
                    = GetPortPixMap( rSalGraphicsData->mpCGrafPort );

                rSalGraphicsData->mnOSStatus = QDErr();

                if (    ( rSalGraphicsData->mnOSStatus     == noErr )
                     && ( rSalGraphicsData->mhGWorldPixMap != NULL  )
                  )
                {
                    // Get the current PixMap state, i.e.,
                    // is it the memory associated with this
                    // PixMap is marked to be moved?

                    rSalGraphicsData->mnGWorldFlags
                        = GetPixelsState( rSalGraphicsData->mhGWorldPixMap );

                    if ( rSalGraphicsData->mnGWorldFlags == noErr )
                    {
                        // if the PixMap is a relocatable block,
                        // then mark it as locked.

                        if ( LockPixels( rSalGraphicsData->mhGWorldPixMap ) )
                        {
                            rSalGraphicsData->mbGWorldPixelsLocked = TRUE;
                        } // if
                        else
                        {
                            rSalGraphicsData->mbGWorldPixelsLocked = FALSE;
                        } // else

                        // Set background color to white on this GWorld

                        SetWhiteBackColor();

                        // Set foreground color to black on this GWorld

                        SetBlackForeColor();

                        // If we get here then we may safely start drawing
                        // to our GWorld

                        bStartGraphics = TRUE;

                        // Now begin to set the clip region

                        if (    ( rSalGraphicsData->mbClipRgnChanged == TRUE )
                             && ( rSalGraphicsData->mhClipRgn        != NULL )
                           )
                        {
                            // Set to the clip region

                            SetClip( rSalGraphicsData->mhClipRgn );

                            // Was there an error after setting the clip region?

                            rSalGraphicsData->mnOSStatus = QDErr();

                            // Set the new status flag for our port

                            rSalGraphicsData->mbClipRgnChanged = FALSE;
                        } // if
                    } // if
                } // if
            } // if
        } // if
    } // if

    return bStartGraphics;
} // BeginGraphics

// -----------------------------------------------------------------------

BOOL EndGraphics ( SalGraphicsDataPtr rSalGraphicsData )
{
    BOOL bEndGraphics = FALSE;

    // Previous to entering this function, was there a QD error?

    if ( rSalGraphicsData->mnOSStatus == noErr )
    {
        // Unlock our CGrafPort

        if (    ( rSalGraphicsData->mhGWorldPixMap != NULL  )
             && ( rSalGraphicsData->mnGWorldFlags  == noErr )
             && ( rSalGraphicsData->mbGWorldPixelsLocked    )
           )
        {
            // Set pixel state to its original state
            // thus unlocking the PixMap

            SetPixelsState( rSalGraphicsData->mhGWorldPixMap,
                            rSalGraphicsData->mnGWorldFlags
                          );

            rSalGraphicsData->mnOSStatus = QDErr();
            rSalGraphicsData->mbGWorldPixelsLocked = FALSE;
        } // if

        // Reset the port's pen to its original attributes

        SetPortPenPixPat( rSalGraphicsData->mpCGrafPort,
                                  rSalGraphicsData->mhPortPenPattern
                                );

        SetPortPenSize( rSalGraphicsData->mpCGrafPort,
                                rSalGraphicsData->maPortPenSize
                              );

        SetPortPenMode( rSalGraphicsData->mpCGrafPort,
                        rSalGraphicsData->mnPortPenMode
                      );

        // Unlock focus on the current NSView

        if ( rSalGraphicsData->mbWindow == TRUE )
        {
            VCLGraphics_UnLockFocusCGrafPort( rSalGraphicsData->mhDC );
        } // if

        // When we get here then the QD port must have changed(?)

        PortChanged( rSalGraphicsData->mpCGrafPort );

        // Flush the QuickDraw buffer

        QDFlushPortBuffer( rSalGraphicsData->mpCGrafPort, NULL );

        // Was there an error after flushing the QuickDraw buffer

        rSalGraphicsData->mnOSStatus = QDErr();

        // Set the new status flag for clip region

        rSalGraphicsData->mbClipRgnChanged = FALSE;

        // If we get here then we have safely written to our GWorld

        bEndGraphics = TRUE;
    } // if

    return bEndGraphics;
} // EndGraphics

// =======================================================================

// =======================================================================

void InitBrush ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->mbBrushTransparent = FALSE;
    rSalGraphicsData->maBrushColor       = GetBlackColor();
} // InitBrush

// -----------------------------------------------------------------------

void InitFont ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->maFontColor = GetBlackColor();
    rSalGraphicsData->mnFontID    = kFontIDGeneva;
    rSalGraphicsData->mnFontSize  = 10;
    rSalGraphicsData->mnFontStyle = normal;
} // InitFont

// -----------------------------------------------------------------------

void InitGWorld ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->mbGWorldPixelsLocked = FALSE;
    rSalGraphicsData->mbGWorldPixelsCopy   = FALSE;
    rSalGraphicsData->mbGWorldPixelsNew    = FALSE;
    rSalGraphicsData->mnGWorldFlags        = noErr;
    rSalGraphicsData->mhGWorldPixMap       = NULL;
} // InitGWorld

// -----------------------------------------------------------------------

void InitPen ( SalGraphicsDataPtr rSalGraphicsData )
{
    long  nMacOSPointSize = sizeof(MacOSPoint);

    rSalGraphicsData->maPenColor       = GetBlackColor();
    rSalGraphicsData->mnPenMode        = patCopy;
    rSalGraphicsData->mbPenTransparent = FALSE;

    rSalGraphicsData->mnPortPenMode    = patCopy;
    rSalGraphicsData->mhPortPenPattern = NULL;

    memset(     &(rSalGraphicsData->maPortPenSize), 0, nMacOSPointSize );
    memset( &(rSalGraphicsData->maPortPenLocation), 0, nMacOSPointSize );
} // InitPen

// -----------------------------------------------------------------------

void InitQD ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->mhDC        = NULL;
    rSalGraphicsData->mpCGrafPort = NULL;
    rSalGraphicsData->mpGWorld    = NULL;
} // InitQD

// -----------------------------------------------------------------------

void InitRegions ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->mhClipRgn        = NULL;
    rSalGraphicsData->mbClipRgnChanged = FALSE;
} // InitRegions

// -----------------------------------------------------------------------

void InitStatusFlags ( SalGraphicsDataPtr rSalGraphicsData )
{
    rSalGraphicsData->mbPrinter  = FALSE;
    rSalGraphicsData->mbVirDev   = FALSE;
    rSalGraphicsData->mbWindow   = FALSE;
    rSalGraphicsData->mbScreen   = FALSE;
    rSalGraphicsData->mnOSStatus = noErr;
} // InitStatusFlags

// =======================================================================

// =======================================================================

