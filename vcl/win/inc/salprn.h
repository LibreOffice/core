/*************************************************************************
 *
 *  $RCSfile: salprn.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:49:31 $
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

#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif

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

// ---------------------
// - WinSalInfoPrinter -
// ---------------------

class WinSalGraphics;

class WinSalInfoPrinter : public SalInfoPrinter
{
public:
    WinSalGraphics*         mpGraphics;             // current Printer graphics
    XubString               maDriverName;           // printer driver name
    XubString               maDeviceName;           // printer device name
    XubString               maPortName;             // printer port name
    ByteString              maDriverNameA;          // printer driver name
    ByteString              maDeviceNameA;          // printer device name
    ByteString              maPortNameA;            // printer port name
    HDC                     mhDC;                   // printer hdc
    BOOL                    mbGraphics;             // is Graphics used
    BOOL                    mbAnsi;

public:
    WinSalInfoPrinter();
    virtual ~WinSalInfoPrinter();

    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual BOOL                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData );
    virtual BOOL                    SetPrinterData( ImplJobSetup* pSetupData );
    virtual BOOL                    SetData( ULONG nFlags, ImplJobSetup* pSetupData );
    virtual void                    GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight );
    virtual ULONG                   GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType );
    virtual ULONG                   GetPaperBinCount( const ImplJobSetup* pSetupData );
    virtual String                  GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin );
    virtual void                    InitPaperFormats( const ImplJobSetup* pSetupData );
    virtual int                 GetLandscapeAngle( const ImplJobSetup* pSetupData );
};

// -----------------
// - WinSalPrinter -
// -----------------

class WinSalPrinter : public SalPrinter
{
public:
    WinSalGraphics*         mpGraphics;             // current Printer graphics
    WinSalInfoPrinter*      mpInfoPrinter;          // pointer to the compatible InfoPrinter
    WinSalPrinter*          mpNextPrinter;          // next printing printer
    HDC                     mhDC;                   // printer hdc
    ULONG                   mnError;                // Error Code
    ULONG                   mnCopies;               // Kopien
    BOOL                    mbCollate;              // Sortierte Kopien
    BOOL                    mbAbort;                // Job Aborted

public:
    WinSalPrinter();
    virtual ~WinSalPrinter();

    virtual BOOL                    StartJob( const XubString* pFileName,
                                              const XubString& rJobName,
                                              const XubString& rAppName,
                                              ULONG nCopies, BOOL bCollate,
                                              ImplJobSetup* pSetupData );
    virtual BOOL                    EndJob();
    virtual BOOL                    AbortJob();
    virtual SalGraphics*            StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData );
    virtual BOOL                    EndPage();
    virtual ULONG                   GetErrorCode();
};

#endif // _SV_SALPRN_H
