/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salprn.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:46:18 $
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
#include <vcl/sv.h>
#endif

#ifndef _SV_SALPRN_HXX
#include <vcl/salprn.hxx>
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
    BOOL                    mbDJPSupported;         // is driver DJP enabled
    BOOL                    mbGraphics;             // is Graphics used

public:
    Os2SalInfoPrinter();
    virtual ~Os2SalInfoPrinter();

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
    virtual DuplexMode          GetDuplexMode( const ImplJobSetup* pSetupData );
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
    BOOL                    mbFirstPage;            // IsFirstPage
    BOOL                    mbAbort;                // JobAborted
    BOOL                    mbPrintDJPSupported;    // is driver PrintDJP enabled (DEVESC_NEWFRAME_WPROP)
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
    BOOL                    mbFirstPage;            // IsFirstPage
    BOOL                    mbAbort;                // JobAborted
    BOOL                    mbPrintDJPSupported;    // is driver PrintDJP enabled (DEVESC_NEWFRAME_WPROP)
    char                    maCommentBuf[33];       // Comment
    char                    maCopyBuf[16];          // Kopien
    //HDC                   mhDC;                   // printer hdc
    //ULONG                 mnError;                // Error Code
    //ULONG                 mnCopies;               // Kopien
    //BOOL                  mbCollate;              // Sortierte Kopien
    //BOOL                  mbAbort;                // Job Aborted

public:
    Os2SalPrinter();
    virtual ~Os2SalPrinter();

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
