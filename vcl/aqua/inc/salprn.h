/*************************************************************************
 *
 *  $RCSfile: salprn.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-01 03:12:44 $
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

#ifndef _SV_SALPRN_H
#define _SV_SALPRN_H

#ifndef _SV_SV_H
#include <sv.h>
#endif

class SalGraphics;
class SalInfoPrinter;

// -----------------
// - SalDriverData -
// -----------------

// WNT3
#define SAL_DRIVERDATA_SYSSIGN              ((ULONG)0x574E5433)
#define SAL_DRIVERDATA_VERSION              1
#define SAL_DEVMODE( pSetupData )           ((LPDEVMODE)((pSetupData->mpDriverData) + (((SalDriverData*)(pSetupData->mpDriverData))->mnDriverOffset)))

#pragma pack( 1 )

struct SalDriverData
{
    ULONG                   mnSysSignature;
    USHORT                  mnVersion;
    USHORT                  mnDriverOffset;
    BYTE                    maDriverData[1];
};

#pragma pack()

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

class SalInfoPrinterData
{
public:
    SalGraphics*            mpGraphics;             // current Printer graphics
    XubString               maDriverName;           // printer driver name
    XubString               maDeviceName;           // printer device name
    XubString               maPortName;             // printer port name
    ByteString              maDriverNameA;          // printer driver name
    ByteString              maDeviceNameA;          // printer device name
    ByteString              maPortNameA;            // printer port name
    VCLVIEW                     mhDC;                   // printer hdc
    BOOL                    mbGraphics;             // is Graphics used
    BOOL                    mbAnsi;
};

// ------------------
// - SalPrinterData -
// ------------------

class SalPrinterData
{
public:
    SalGraphics*            mpGraphics;             // current Printer graphics
    SalInfoPrinter*         mpInfoPrinter;          // pointer to the compatible InfoPrinter
    SalPrinter*             mpNextPrinter;          // next printing printer
    VCLVIEW                     mhDC;                   // printer hdc
    ULONG                   mnError;                // Error Code
    ULONG                   mnCopies;               // Kopien
    BOOL                    mbCollate;              // Sortierte Kopien
    BOOL                    mbAbort;                // Job Aborted
};

#endif // _SV_SALPRN_H
