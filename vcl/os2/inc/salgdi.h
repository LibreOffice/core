/*************************************************************************
 *
 *  $RCSfile: salgdi.h,v $
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

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

#ifndef _SV_SV_H
#include <sv.h>
#endif

// -------------------
// - SalGraphicsData -
// -------------------

class SalGraphicsData
{
public:
    HPS                     mhPS;               // HPS
    HDC                     mhDC;               // HDC
    HWND                    mhWnd;              // HWND
    LONG                    mnHeight;           // Height of frame Window
    ULONG                   mnClipElementCount; // number of clip rects in clip rect array
    RECTL*                  mpClipRectlAry;     // clip rect array
    ULONG                   mnFontMetricCount;  // number of entries in the font list
    PFONTMETRICS            mpFontMetrics;      // cached font list
    LONG                    mnOrientationX;     // X-Font orientation
    LONG                    mnOrientationY;     // Y-Font orientation
    BOOL                    mbFontIsOutline;    // is outline font
    BOOL                    mbFontIsFixed;      // is fixed font
    BOOL                    mbLine;             // draw lines
    BOOL                    mbFill;             // fill areas
    BOOL                    mbPrinter;          // is Printer
    BOOL                    mbVirDev;           // is VirDev
    BOOL                    mbWindow;           // is Window
    BOOL                    mbScreen;           // is Screen compatible
    BOOL                    mbXORMode;          // _every_ output with RasterOp XOR
};

// Init/Deinit Graphics
void ImplSalInitGraphics( SalGraphicsData* mpData );
void ImplSalDeInitGraphics( SalGraphicsData* mpData );

// -----------
// - Defines -
// -----------

#define RGBCOLOR(r,g,b)     ((ULONG)(((BYTE)(b)|((USHORT)(g)<<8))|(((ULONG)(BYTE)(r))<<16)))
#define TY( y )             (maGraphicsData.mnHeight-(y)-1)

#endif // _SV_SALGDI_H
