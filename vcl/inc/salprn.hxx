/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SV_SALPRN_HXX
#define _SV_SALPRN_HXX

#include <rtl/ustring.hxx>

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/prntypes.hxx>

#include <vector>

class SalGraphics;
class SalFrame;
struct ImplJobSetup;
namespace vcl { class PrinterController; }

struct VCL_PLUGIN_PUBLIC SalPrinterQueueInfo
{
    OUString               maPrinterName;
    OUString               maDriver;
    OUString               maLocation;
    OUString               maComment;
    sal_uLong                   mnStatus;
    sal_uLong                   mnJobs;
    OUString*              mpSysData;

                            SalPrinterQueueInfo();
                            ~SalPrinterQueueInfo();
};

class VCL_PLUGIN_PUBLIC SalInfoPrinter
{
public:
    std::vector< PaperInfo  >           m_aPaperFormats;    // all printer supported formats
    bool                                m_bPapersInit;      // set to true after InitPaperFormats

    SalInfoPrinter() : m_bPapersInit( false ) {}
    virtual ~SalInfoPrinter();

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*            GetGraphics() = 0;
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics ) = 0;

    virtual sal_Bool                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData ) = 0;
    // This function set the driver data and
    // set the new indepen data in pSetupData
    virtual sal_Bool                    SetPrinterData( ImplJobSetup* pSetupData ) = 0;
    // This function merged the indepen driver data
    // and set the new indepen data in pSetupData
    // Only the data must changed, where the bit
    // in nFlags is set
    virtual sal_Bool                    SetData( sal_uLong nFlags, ImplJobSetup* pSetupData ) = 0;

    virtual void                    GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight ) = 0;
    virtual sal_uLong                   GetCapabilities( const ImplJobSetup* pSetupData, sal_uInt16 nType ) = 0;
    virtual sal_uLong                   GetPaperBinCount( const ImplJobSetup* pSetupData ) = 0;
    virtual OUString                  GetPaperBinName( const ImplJobSetup* pSetupData, sal_uLong nPaperBin ) = 0;
    // fills m_aPaperFormats and sets m_bPapersInit to true
    virtual void                    InitPaperFormats( const ImplJobSetup* pSetupData ) = 0;
    // returns angle that a landscape page will be turned counterclockwise wrt to portrait
    virtual int                 GetLandscapeAngle( const ImplJobSetup* pSetupData ) = 0;
};

class VCL_PLUGIN_PUBLIC SalPrinter
{
public:                     // public for Sal Implementation
    SalPrinter() {}
    virtual ~SalPrinter();

    virtual sal_Bool                    StartJob( const OUString* pFileName,
                                              const OUString& rJobName,
                                              const OUString& rAppName,
                                              sal_uLong nCopies,
                                              bool bCollate,
                                              bool bDirect,
                                              ImplJobSetup* pSetupData ) = 0;

    // implement for pull model print systems only,
    // default implementations (see salvtables.cxx) just returns sal_False
    virtual sal_Bool                    StartJob( const OUString* pFileName,
                                              const OUString& rJobName,
                                              const OUString& rAppName,
                                              ImplJobSetup* pSetupData,
                                              vcl::PrinterController& rController );

    virtual sal_Bool                    EndJob() = 0;
    virtual sal_Bool                    AbortJob() = 0;
    virtual SalGraphics*            StartPage( ImplJobSetup* pSetupData, sal_Bool bNewJobData ) = 0;
    virtual sal_Bool                    EndPage() = 0;
    virtual sal_uLong                   GetErrorCode() = 0;

};

#endif // _SV_SALPRN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
