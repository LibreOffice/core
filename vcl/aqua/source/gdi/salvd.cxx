/*************************************************************************
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: bmahbod $ $Date: 2001-02-21 20:48:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *       - GNU Lesser General Public License Version 2.1
 *       - Sun Industry Standards Source License Version 1.1
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

#define _SV_SALVD_CXX

#ifndef _SV_SALCONST_H
#include <salconst.h>
#endif

#ifndef _SV_SALCOLORUTILS_HXX
#include <salcolorutils.hxx>
#endif

#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

// =======================================================================

// =======================================================================

static BOOL InitVirtualDeviceGWorld ( SalVirDevDataPtr rSalVirDevData )
{
    BOOL  bVirtualDeviceGWorldInited = FALSE;

    if (    ( rSalVirDevData             != NULL )
         && ( rSalVirDevData->mpGraphics != NULL )
       )
    {
        Rect         aBoundsRect;
        short        nRectLeft   = 0;
        short        nRectTop    = 0;
        short        nRectRight  = rSalVirDevData->mnWidth;
        short        nRectBottom = rSalVirDevData->mnHeight;
        short        nPixelDepth = rSalVirDevData->mnBitCount;
        GWorldPtr    pGWorld     = NULL;
        CTabHandle   hCTable     = NULL;
        GDHandle     hGDevice    = NULL;
        GWorldFlags  nFlags      = noNewDevice;
        OSStatus     nOSStatus   = noErr;

        // Set the dimensions of the GWorldPtr

        MacSetRect( &aBoundsRect, nRectLeft, nRectTop, nRectRight, nRectBottom );

        // Create the offscreen graphics context

        nOSStatus = NewGWorld( &pGWorld,
                                nPixelDepth,
                               &aBoundsRect,
                                hCTable,
                                hGDevice,
                                nFlags
                             );

        // If NewGWorld failed, try again with different flags

        if ( nOSStatus != noErr )
        {
            nFlags = noErr;

            nOSStatus = NewGWorld( &pGWorld,
                                    nPixelDepth,
                                   &aBoundsRect,
                                    hCTable,
                                    hGDevice,
                                    nFlags
                                 );
        } // if

        if (    ( nOSStatus == noErr )
             && ( pGWorld   != NULL  )
           )
        {
            // Lock the virtual GWorld's port bits

            rSalVirDevData->mpGraphics->maGraphicsData.mnOSStatus
                = LockPortBits( pGWorld );

            if ( rSalVirDevData->mpGraphics->maGraphicsData.mnOSStatus == noErr )
            {
                // Initialize the virtual graph port

                rSalVirDevData->mpGraphics->maGraphicsData.mpCGrafPort
                    = pGWorld;

                rSalVirDevData->mpGraphics->maGraphicsData.mpGWorld
                    = pGWorld;

                // Initialize virtual port's GWorld attributes

                rSalVirDevData->mpGraphics->maGraphicsData.mhGWorldPixMap
                    = GetGWorldPixMap( pGWorld );

                // Unlock virtual GWorld's port bits

                UnlockPortBits( pGWorld );
            } // if

            // Initialize virtual port's GWorld attributes

            rSalVirDevData->mpGraphics->maGraphicsData.mbGWorldPixelsLocked = FALSE;
            rSalVirDevData->mpGraphics->maGraphicsData.mbGWorldPixelsCopy   = FALSE;
            rSalVirDevData->mpGraphics->maGraphicsData.mbGWorldPixelsNew    = FALSE;
            rSalVirDevData->mpGraphics->maGraphicsData.mnGWorldFlags        = noErr;

            // Initialize the virtual port's brush attributes

            rSalVirDevData->mpGraphics->maGraphicsData.mbBrushTransparent = FALSE;
            rSalVirDevData->mpGraphics->maGraphicsData.maBrushColor       = GetBlackColor( );

            // Initialize the virtual port's font attributes

            rSalVirDevData->mpGraphics->maGraphicsData.maFontColor = GetBlackColor( );
            rSalVirDevData->mpGraphics->maGraphicsData.mnFontID    = kFontIDGeneva;
            rSalVirDevData->mpGraphics->maGraphicsData.mnFontSize  = 10;
            rSalVirDevData->mpGraphics->maGraphicsData.mnFontStyle = normal;

            // Initialize virtual port's clip regions

            rSalVirDevData->mpGraphics->maGraphicsData.mhClipRgn        = NULL;
            rSalVirDevData->mpGraphics->maGraphicsData.mbClipRgnChanged = FALSE;

            // Initilaize virtual port's status flags

            rSalVirDevData->mpGraphics->maGraphicsData.mbPrinter = FALSE;
            rSalVirDevData->mpGraphics->maGraphicsData.mbVirDev  = TRUE;
            rSalVirDevData->mpGraphics->maGraphicsData.mbWindow  = FALSE;
            rSalVirDevData->mpGraphics->maGraphicsData.mbScreen  = TRUE;

            bVirtualDeviceGWorldInited = TRUE;
        } // if
    } // if

    return bVirtualDeviceGWorldInited;
} //InitVirtualDeviceGWorld

// =======================================================================

// =======================================================================

SalVirtualDevice::SalVirtualDevice()
{
    maVirDevData.mpGraphics = NULL;
    maVirDevData.mnBitCount = 0;
    maVirDevData.mnWidth    = 0;
    maVirDevData.mnHeight   = 0;
    maVirDevData.mbGraphics = FALSE;
} // Constructor

// -----------------------------------------------------------------------

SalVirtualDevice::~SalVirtualDevice()
{
    if ( maVirDevData.mpGraphics != NULL )
    {
        // Delete exisiting clip regions, offscreen graphic world,
        // and its associated colour graph port

        delete maVirDevData.mpGraphics;
    } // if
} // Destructor

// -----------------------------------------------------------------------

SalGraphics* SalVirtualDevice::GetGraphics()
{
    if ( maVirDevData.mbGraphics )
    {
        return NULL;
    } // if

    if ( !maVirDevData.mpGraphics )
    {
        maVirDevData.mpGraphics = new SalGraphics;

        maVirDevData.mbGraphics = InitVirtualDeviceGWorld( &maVirDevData );

        if ( !maVirDevData.mbGraphics )
        {
            delete maVirDevData.mpGraphics;
            maVirDevData.mpGraphics = NULL;
        } // if
    } // if

    return maVirDevData.mpGraphics;
} // SalVirtualDevice::GetGraphics

// -----------------------------------------------------------------------

void SalVirtualDevice::ReleaseGraphics( SalGraphics *pGraphics )
{
    maVirDevData.mbGraphics = FALSE;
} // SalVirtualDevice::ReleaseGraphics

// -----------------------------------------------------------------------

BOOL SalVirtualDevice::SetSize( long nDX, long nDY )
{
    BOOL bSizeSet = FALSE;

    // If we have already created a graphics context, dispose of it,
    // by deleting exisiting clip regions, offscreen graphic worlds,
    // and its associated colour graph port

    if (    (    ( maVirDevData.mpGraphics->maGraphicsData.mbGWorldPixelsCopy == TRUE )
              || ( maVirDevData.mpGraphics->maGraphicsData.mbGWorldPixelsNew  == TRUE )
            )
         && ( maVirDevData.mpGraphics->maGraphicsData.mhGWorldPixMap != NULL )
           )
    {
        DisposePixMap( maVirDevData.mpGraphics->maGraphicsData.mhGWorldPixMap );
    } // if

    if ( maVirDevData.mpGraphics->maGraphicsData.mhClipRgn != NULL )
    {
        DisposeRgn( maVirDevData.mpGraphics->maGraphicsData.mhClipRgn );
    } // if

    if ( maVirDevData.mpGraphics->maGraphicsData.mpCGrafPort != NULL )
    {
        DisposeGWorld( maVirDevData.mpGraphics->maGraphicsData.mpCGrafPort );

        maVirDevData.mpGraphics->maGraphicsData.mpGWorld    = NULL;
        maVirDevData.mpGraphics->maGraphicsData.mpCGrafPort = NULL;
    } // if

    // Create the offscreen graphics context

    maVirDevData.mnWidth  = nDX;
    maVirDevData.mnHeight = nDY;

    bSizeSet = InitVirtualDeviceGWorld( &maVirDevData );

    return bSizeSet;
} // SalVirtualDevice::SetSize

// =======================================================================

// =======================================================================

