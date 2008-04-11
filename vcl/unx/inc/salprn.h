/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salprn.h,v $
 * $Revision: 1.18 $
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

#include <psprint/jobdata.hxx>
#include <psprint/printergfx.hxx>
#include <psprint/printerjob.hxx>
#include <vcl/salprn.hxx>

class PspGraphics;

class PspSalInfoPrinter : public SalInfoPrinter
{
public:
    PspGraphics*            m_pGraphics;
    psp::JobData            m_aJobData;
    psp::PrinterGfx         m_aPrinterGfx;

    PspSalInfoPrinter();
    virtual ~PspSalInfoPrinter();

    // overload all pure virtual methods
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

class PspSalPrinter : public SalPrinter
{
public:
    String                  m_aFileName;
    String                  m_aTmpFile;
    String                  m_aFaxNr;
    bool                    m_bFax:1;
    bool                    m_bPdf:1;
    bool                    m_bSwallowFaxNo:1;
    PspGraphics*            m_pGraphics;
    psp::PrinterJob         m_aPrintJob;
    psp::JobData            m_aJobData;
    psp::PrinterGfx         m_aPrinterGfx;
    ULONG                   m_nCopies;
    SalInfoPrinter*         m_pInfoPrinter;

    PspSalPrinter( SalInfoPrinter* );
    virtual ~PspSalPrinter();

    // overload all pure virtual methods
    using SalPrinter::StartJob;
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

class Timer;

namespace vcl_sal {
class VCL_DLLPUBLIC PrinterUpdate
{
    static Timer*           pPrinterUpdateTimer;
    static int              nActiveJobs;

    static void doUpdate();
    DECL_STATIC_LINK( PrinterUpdate, UpdateTimerHdl, void* );
public:
    static void update();
    static void jobStarted() { nActiveJobs++; }
    static void jobEnded();
};
}

#endif // _SV_SALPRN_H


