/*************************************************************************
 *
 *  $RCSfile: salprn.h,v $
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

#ifndef _SV_SALPRN_H
#define _SV_SALPRN_H

#ifndef _SV_SV_H
#include <sv.h>
#endif

class SalGraphics;
class SalInfoPrinter;

struct ImplFormInfo;
typedef ImplFormInfo* PIMPLFORMINFO;
struct ImplTrayInfo;
typedef ImplTrayInfo* PIMPLTRAYINFO;

// ----------------------
// - SalInfoPrinterData -
// ----------------------

class SalInfoPrinterData
{
public:
    SalGraphics*            mpGraphics;             // Graphics
    HDC                     mhDC;                   // printer hdc
    HPS                     mhPS;                   // printer hps
    String                  maPrinterName;          // pszPrinters
    String                  maName;                 // pszName bzw. LogAdress
    String                  maDriverName;           // pszDriverName nach .
    String                  maDeviceName;           // pszDriverName bis .
    String                  maJobSetupDeviceName;   // DeviceName aus pDriverData
    PIMPLFORMINFO*          mpFormArray;            // PaperForm-Names
    USHORT                  mnFormCount;            // PaperForm-Count
    PIMPLTRAYINFO*          mpTrayArray;            // PaperTray-Names
    USHORT                  mnTrayCount;            // PaperTray-Count
    BOOL                    mbDJPSupported;         // is driver DJP enabled
    BOOL                    mbGraphics;             // is Graphics used
};

// ------------------
// - SalPrinterData -
// ------------------

class SalPrinterData
{
public:
    SalGraphics*            mpGraphics;             // current Printer graphics
    SalInfoPrinter*         mpInfoPrinter;          // pointer to the compatible InfoPrinter
    HDC                     mhDC;                   // printer hdc
    HPS                     mhPS;                   // printer hps
    ULONG                   mnError;                // Error Code
    BOOL                    mbFirstPage;            // IsFirstPage
    BOOL                    mbAbort;                // JobAborted
    BOOL                    mbPrintDJPSupported;    // is driver PrintDJP enabled (DEVESC_NEWFRAME_WPROP)
    char                    maCommentBuf[33];       // Comment
    char                    maCopyBuf[10];          // Kopien
};

#endif // _SV_SALPRN_H
