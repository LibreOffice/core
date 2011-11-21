/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_GENERIC_PRN_H
#define _SV_GENERIC_PRN_H

#include "vcl/jobdata.hxx"
#include "generic/printergfx.hxx"
#include "generic/printerjob.hxx"
#include "salprn.hxx"
#include "vclpluginapi.h"

class GenPspGraphics;
class VCL_DLLPUBLIC PspSalInfoPrinter : public SalInfoPrinter
{
public:
    GenPspGraphics*         m_pGraphics;
    psp::JobData            m_aJobData;
    psp::PrinterGfx         m_aPrinterGfx;

    PspSalInfoPrinter();
    virtual ~PspSalInfoPrinter();

    // overload all pure virtual methods
    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual sal_Bool                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData );
    virtual sal_Bool                    SetPrinterData( ImplJobSetup* pSetupData );
    virtual sal_Bool                    SetData( sal_uIntPtr nFlags, ImplJobSetup* pSetupData );
    virtual void                    GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight );
    virtual sal_uIntPtr                 GetCapabilities( const ImplJobSetup* pSetupData, sal_uInt16 nType );
    virtual sal_uIntPtr                 GetPaperBinCount( const ImplJobSetup* pSetupData );
    virtual rtl::OUString                  GetPaperBinName( const ImplJobSetup* pSetupData, sal_uIntPtr nPaperBin );
    virtual void                    InitPaperFormats( const ImplJobSetup* pSetupData );
    virtual int                 GetLandscapeAngle( const ImplJobSetup* pSetupData );
};

class VCL_DLLPUBLIC PspSalPrinter : public SalPrinter
{
public:
    rtl::OUString                  m_aFileName;
    rtl::OUString                  m_aTmpFile;
    rtl::OUString                  m_aFaxNr;
    bool                    m_bFax:1;
    bool                    m_bPdf:1;
    bool                    m_bSwallowFaxNo:1;
    bool                    m_bIsPDFWriterJob:1;
    GenPspGraphics*         m_pGraphics;
    psp::PrinterJob         m_aPrintJob;
    psp::JobData            m_aJobData;
    psp::PrinterGfx         m_aPrinterGfx;
    sal_uIntPtr                 m_nCopies;
    bool                    m_bCollate;
    SalInfoPrinter*         m_pInfoPrinter;

    PspSalPrinter( SalInfoPrinter *pPrinter );
    virtual ~PspSalPrinter();

    // overload all pure virtual methods
    virtual sal_Bool                    StartJob( const rtl::OUString* pFileName,
                                              const rtl::OUString& rJobName,
                                              const rtl::OUString& rAppName,
                                              sal_uIntPtr nCopies,
                                              bool bCollate,
                                              bool bDirect,
                                              ImplJobSetup* pSetupData );
    virtual sal_Bool                    StartJob( const rtl::OUString*,
                                              const rtl::OUString&,
                                              const rtl::OUString&,
                                              ImplJobSetup*,
                                              vcl::PrinterController& i_rController );
    virtual sal_Bool                    EndJob();
    virtual sal_Bool                    AbortJob();
    virtual SalGraphics*            StartPage( ImplJobSetup* pSetupData, sal_Bool bNewJobData );
    virtual sal_Bool                    EndPage();
    virtual sal_uIntPtr                 GetErrorCode();
};

#endif // _SV_SALPRN_H


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
