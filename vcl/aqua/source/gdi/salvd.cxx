/*************************************************************************
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pluby $ $Date: 2000-12-24 19:40:29 $
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

#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

// =======================================================================

SalVirtualDevice::SalVirtualDevice()
{
    maVirDevData.mpGraphics = NULL;
    maVirDevData.mnBitCount = 0;
    maVirDevData.mnWidth = 0;
    maVirDevData.mnHeight = 0;
    maVirDevData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

SalVirtualDevice::~SalVirtualDevice()
{
    if ( maVirDevData.mpGraphics )
        delete maVirDevData.mpGraphics;
}

// -----------------------------------------------------------------------

SalGraphics* SalVirtualDevice::GetGraphics()
{
    if ( maVirDevData.mbGraphics )
        return NULL;

    if ( !maVirDevData.mpGraphics )
    {
        maVirDevData.mpGraphics = new SalGraphics;
        GWorldPtr pGWorld = NULL;
        Rect aRect;
        OSStatus aQDStatus = noErr;

        // Set the dimensions of the GWorldPtr
        MacSetRect( &aRect, 0, 0, maVirDevData.mnWidth, maVirDevData.mnHeight );

        // Create the offscreen graphics context
        aQDStatus = NewGWorld( &pGWorld, maVirDevData.mnBitCount, &aRect, NULL,
            NULL, 0);

        if ( aQDStatus == noErr );
        {
            maVirDevData.mpGraphics->maGraphicsData.mpCGrafPort = (CGrafPtr)pGWorld;
            maVirDevData.mpGraphics->maGraphicsData.mbPrinter = FALSE;
            maVirDevData.mpGraphics->maGraphicsData.mbVirDev  = TRUE;
            maVirDevData.mpGraphics->maGraphicsData.mbWindow  = FALSE;
            maVirDevData.mpGraphics->maGraphicsData.mbScreen  = TRUE;
        }
    }

    maVirDevData.mbGraphics = TRUE;

    return maVirDevData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalVirtualDevice::ReleaseGraphics( SalGraphics *pGraphics )
{
    maVirDevData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL SalVirtualDevice::SetSize( long nDX, long nDY )
{
    GWorldPtr pGWorld = NULL;
    Rect aRect;
    OSStatus aQDStatus = noErr;

    maVirDevData.mnWidth = nDX;
    maVirDevData.mnHeight = nDY;

    // Set the dimensions of the GWorldPtr
    MacSetRect( &aRect, 0, 0, maVirDevData.mnWidth, maVirDevData.mnHeight );

    // If we have already created a graphics context, dispose of it
    if ( maVirDevData.mpGraphics->maGraphicsData.mpCGrafPort )
    {
        DisposeGWorld( maVirDevData.mpGraphics->maGraphicsData.mpCGrafPort );
        maVirDevData.mpGraphics->maGraphicsData.mpCGrafPort = NULL;
    }

    // Create the offscreen graphics context
    aQDStatus = NewGWorld( &pGWorld, maVirDevData.mnBitCount, &aRect, NULL,
        NULL, 0);
    if ( aQDStatus == noErr )
        maVirDevData.mpGraphics->maGraphicsData.mpCGrafPort = (CGrafPtr)pGWorld;
    else
        return FALSE;

    return TRUE;
}
