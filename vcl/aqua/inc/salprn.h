/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salprn.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:36:03 $
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

#ifndef _SV_SALPRN_H
#define _SV_SALPRN_H

#ifndef _SV_SV_H
#include <sv.h>
#endif


class SalGraphics;
class SalInfoPrinter;

// -------------------
// - SalDriverData -
// -------------------

struct SalDriverData
{
    ULONG                   mnSysSignature;
    USHORT                  mnVersion;
    USHORT                  mnDriverOffset;
    BYTE                    maDriverData[1];
};

// -------------------
// - SalSysQueueData -
// -------------------

struct SalSysQueueData
{
    XubString               maDriverName;           // printer driver name
    XubString               maDeviceName;           // printer device name
    XubString               maPortName;             // printer port name
    ByteString              maDriverNameA;          // printer driver name
    ByteString              maDeviceNameA;          // printer device name
    ByteString              maPortNameA;            // printer port name
    BOOL                    mbAnsi;                 // TRUE - use A functions
};

// ----------------------
// - SalInfoPrinterData -
// ----------------------

struct SalInfoPrinterData
{
    SalGraphics*            mpGraphics;             // current Printer graphics
    XubString               maDriverName;           // printer driver name
    XubString               maDeviceName;           // printer device name
    XubString               maPortName;             // printer port name
    ByteString              maDriverNameA;          // printer driver name
    ByteString              maDeviceNameA;          // printer device name
    ByteString              maPortNameA;            // printer port name
    VCLVIEW                 mhDC;                   // printer hdc
    BOOL                    mbGraphics;             // is Graphics used
    BOOL                    mbAnsi;
};

// ------------------
// - SalPrinterData -
// ------------------

struct SalPrinterData
{
    SalGraphics*            mpGraphics;             // current Printer graphics
    SalInfoPrinter*         mpInfoPrinter;          // pointer to the compatible InfoPrinter
};

#endif // _SV_SALPRN_H
