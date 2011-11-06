/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


