/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SALPRN_H
#define _SV_SALPRN_H

#include <salprn.hxx>

class SalGraphics;
class SalInfoPrinter;

struct ImplFormInfo;
typedef ImplFormInfo* PIMPLFORMINFO;
struct ImplTrayInfo;
typedef ImplTrayInfo* PIMPLTRAYINFO;

// ----------------------
// - SalInfoPrinterData -
// ----------------------

//class SalInfoPrinterData
class Os2SalInfoPrinter : public SalInfoPrinter
{
public:
    Os2SalGraphics*         mpGraphics;             // Graphics
    HDC                     mhDC;                   // printer hdc
    HPS                     mhPS;                   // printer hps
    ByteString                  maPrinterName;          // pszPrinters
    ByteString                  maName;                 // pszName bzw. LogAdress
    ByteString                  maDriverName;           // pszDriverName nach .
    ByteString                  maDeviceName;           // pszDriverName bis .
    ByteString                  maJobSetupDeviceName;   // DeviceName aus pDriverData
    PIMPLFORMINFO*          mpFormArray;            // PaperForm-Names
    USHORT                  mnFormCount;            // PaperForm-Count
    PIMPLTRAYINFO*          mpTrayArray;            // PaperTray-Names
    USHORT                  mnTrayCount;            // PaperTray-Count
    sal_Bool                    mbDJPSupported;         // is driver DJP enabled
    sal_Bool                    mbGraphics;             // is Graphics used

public:
    Os2SalInfoPrinter();
    virtual ~Os2SalInfoPrinter();

    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual sal_Bool                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData );
    virtual sal_Bool                    SetPrinterData( ImplJobSetup* pSetupData );
    virtual sal_Bool                    SetData( ULONG nFlags, ImplJobSetup* pSetupData );
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

// ------------------
// - SalPrinterData -
// ------------------

class SalPrinterData00
{
public:
    Os2SalGraphics*         mpGraphics;             // current Printer graphics
    SalInfoPrinter*         mpInfoPrinter;          // pointer to the compatible InfoPrinter
    HDC                     mhDC;                   // printer hdc
    HPS                     mhPS;                   // printer hps
    ULONG                   mnError;                // Error Code
    sal_Bool                    mbFirstPage;            // IsFirstPage
    sal_Bool                    mbAbort;                // JobAborted
    sal_Bool                    mbPrintDJPSupported;    // is driver PrintDJP enabled (DEVESC_NEWFRAME_WPROP)
    char                    maCommentBuf[33];       // Comment
    char                    maCopyBuf[10];          // Kopien
};

// -----------------
// - Os2SalPrinter -
// -----------------

class Os2SalPrinter : public SalPrinter
{
public:
    Os2SalGraphics*         mpGraphics;             // current Printer graphics
    Os2SalInfoPrinter*      mpInfoPrinter;          // pointer to the compatible InfoPrinter
    Os2SalPrinter*          mpNextPrinter;          // next printing printer
    HDC                     mhDC;                   // printer hdc
    HPS                     mhPS;                   // printer hps
    ULONG                   mnError;                // Error Code
    sal_Bool                    mbFirstPage;            // IsFirstPage
    sal_Bool                    mbAbort;                // JobAborted
    sal_Bool                    mbPrintDJPSupported;    // is driver PrintDJP enabled (DEVESC_NEWFRAME_WPROP)
    char                    maCommentBuf[33];       // Comment
    char                    maCopyBuf[16];          // Kopien
    //HDC                   mhDC;                   // printer hdc
    //ULONG                 mnError;                // Error Code
    //ULONG                 mnCopies;               // Kopien
    //sal_Bool                  mbCollate;              // Sortierte Kopien
    //sal_Bool                  mbAbort;                // Job Aborted

public:
    Os2SalPrinter();
    virtual ~Os2SalPrinter();

    virtual sal_Bool                    StartJob( const XubString* pFileName,
                                              const XubString& rJobName,
                                              const XubString& rAppName,
                                              ULONG nCopies,
                                              bool bCollate,
                                              bool bDirect,
                                              ImplJobSetup* pSetupData );
    virtual sal_Bool                    EndJob();
    virtual sal_Bool                    AbortJob();
    virtual SalGraphics*            StartPage( ImplJobSetup* pSetupData, sal_Bool bNewJobData );
    virtual sal_Bool                    EndPage();
    virtual ULONG                   GetErrorCode();
};

#endif // _SV_SALPRN_H
