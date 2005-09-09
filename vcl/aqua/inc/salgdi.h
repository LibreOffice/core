/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salgdi.h,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:34:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#ifndef _SV_SV_H
    #include <sv.h>
#endif

#ifndef _SV_VCLWINDOW_H
    #include <VCLWindow.h>
#endif

// -------------------
// - Structures -
// -------------------

struct SalGraphicsData
{
    // NSView and NSWindow

    VCLVIEW         mhDC;                   // VCLVIEW

    // QuickDraw graph port, offscreen graphic world, and graphic device handle

    CGrafPtr        mpCGrafPort;            // QD color graphics port
    GWorldPtr       mpGWorld;               // QD offscreen GWorld
    GDHandle        mhGDevice;              // QD GDevice

    // Graph port pixels, state and flags

    BOOL            mbGWorldPixelsLocked;   // GWorld pixels locked?
    BOOL            mbGWorldPixelsCopy;     // GWorld pixels was a copy of the original?
    BOOL            mbGWorldPixelsNew;      // GWorld pixels is brand new?
    GWorldFlags     mnGWorldFlags;          // GWorld pixels status flags
    PixMapHandle    mhGWorldPixMap;         // GWorld pixels

    // Clip region

    BOOL            mbClipRgnChanged;       // Did the clip region change?
    RgnHandle       mhClipRgn;              // Clip Region Handle

    // Font attributes

    short           mnFontID;               // Mac FontFamilyId
    short           mnFontSize;             // Mac Font Size
    RGBColor        maFontColor;            // Text Color
    Style           mnFontStyle;            // Mac Font Style

    // Pen attributes and status

    BOOL            mbPenTransparent;       // Is pen transparent?
    SInt32          mnPenMode;              // Pen Mode
    RGBColor        maPenColor;             // Pen Color

    // Port's pen attributes

    SInt32          mnPortPenMode;          // Port's pen mode
    MacOSPoint      maPortPenSize;          // Port's pen size;
    MacOSPoint      maPortPenLocation;      // Port's pen location
    PixPatHandle    mhPortPenPattern;       // Port's pen pattern

    // Brush attributes and status

    BOOL            mbBrushTransparent;     // Is brush transparent?
    RGBColor        maBrushColor;           // Brush Color

    // Miscellaneous status flags

    BOOL            mbPrinter;              // Is a printer available?
    BOOL            mbVirDev;               // Is a virtual device available?
    BOOL            mbWindow;               // Is a window availble?
    BOOL            mbScreen;               // Is this screen compatiable?
    OSStatus        mnOSStatus;             // The current MacOS error
};

typedef struct SalGraphicsData   SalGraphicsData;
typedef SalGraphicsData         *SalGraphicsDataPtr;
typedef SalGraphicsDataPtr      *SalGraphicsDataHandle;

#endif // _SV_SALGDI_H
