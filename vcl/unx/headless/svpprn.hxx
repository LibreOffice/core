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

#ifndef _SVP_SVPPRN_HXX
#define _SVP_SVPPRN_HXX

#include "vcl/jobdata.hxx"

#include "printergfx.hxx"
#include "printerjob.hxx"
#include "salprn.hxx"

#include "vclpluginapi.h"

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
    virtual sal_Bool                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData );
    virtual sal_Bool                    SetPrinterData( ImplJobSetup* pSetupData );
    virtual sal_Bool                    SetData( sal_uLong nFlags, ImplJobSetup* pSetupData );
    virtual void                    GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight );
    virtual sal_uLong                   GetCapabilities( const ImplJobSetup* pSetupData, sal_uInt16 nType );
    virtual sal_uLong                   GetPaperBinCount( const ImplJobSetup* pSetupData );
    virtual String                  GetPaperBinName( const ImplJobSetup* pSetupData, sal_uLong nPaperBin );
    virtual void                    InitPaperFormats( const ImplJobSetup* pSetupData );
    virtual int                 GetLandscapeAngle( const ImplJobSetup* pSetupData );
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
    sal_uLong                   m_nCopies;
    bool                    m_bCollate;
    SalInfoPrinter*         m_pInfoPrinter;

    PspSalPrinter( SalInfoPrinter* );
    virtual ~PspSalPrinter();

    // overload all pure virtual methods
    using SalPrinter::StartJob;
    virtual sal_Bool                    StartJob( const XubString* pFileName,
                                              const XubString& rJobName,
                                              const XubString& rAppName,
                                              sal_uLong nCopies,
                                              bool bCollate,
                                              bool bDirect,
                                              ImplJobSetup* pSetupData );
    virtual sal_Bool                    EndJob();
    virtual sal_Bool                    AbortJob();
    virtual SalGraphics*            StartPage( ImplJobSetup* pSetupData, sal_Bool bNewJobData );
    virtual sal_Bool                    EndPage();
    virtual sal_uLong                   GetErrorCode();
};

class Timer;

namespace vcl_sal {
class VCLPLUG_SVP_PUBLIC PrinterUpdate
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

#endif // _SVP_SVPPRN_HXX


