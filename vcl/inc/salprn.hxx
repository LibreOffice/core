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

#ifndef INCLUDED_VCL_INC_SALPRN_HXX
#define INCLUDED_VCL_INC_SALPRN_HXX

#include <vcl/prntypes.hxx>
#include <vcl/dllapi.h>

#include "salptype.hxx"

#include <vector>

class SalGraphics;
class SalFrame;
class ImplJobSetup;
namespace vcl { class PrinterController; }

struct VCL_PLUGIN_PUBLIC SalPrinterQueueInfo
{
    OUString               maPrinterName;
    OUString               maDriver;
    OUString               maLocation;
    OUString               maComment;
    PrintQueueFlags        mnStatus;
    sal_uLong              mnJobs;
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
    virtual SalGraphics*            AcquireGraphics() = 0;
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics ) = 0;

    virtual bool                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData ) = 0;
    // This function set the driver data and
    // set the new indepen data in pSetupData
    virtual bool                    SetPrinterData( ImplJobSetup* pSetupData ) = 0;
    // This function merged the indepen driver data
    // and set the new indepen data in pSetupData
    // Only the data must changed, where the bit
    // in nFlags is set
    virtual bool                    SetData( JobSetFlags nFlags, ImplJobSetup* pSetupData ) = 0;

    virtual void                    GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight ) = 0;
    virtual sal_uInt32              GetCapabilities( const ImplJobSetup* pSetupData, PrinterCapType nType ) = 0;
    virtual sal_uInt16              GetPaperBinCount( const ImplJobSetup* pSetupData ) = 0;
    virtual OUString                GetPaperBinName( const ImplJobSetup* pSetupData, sal_uInt16 nPaperBin ) = 0;
    // fills m_aPaperFormats and sets m_bPapersInit to true
    virtual void                    InitPaperFormats( const ImplJobSetup* pSetupData ) = 0;
    // returns angle that a landscape page will be turned counterclockwise wrt to portrait
    virtual int                     GetLandscapeAngle( const ImplJobSetup* pSetupData ) = 0;
};

class VCL_PLUGIN_PUBLIC SalPrinter
{
public:
    SalPrinter() {}
    virtual ~SalPrinter();

    virtual bool                    StartJob( const OUString* pFileName,
                                              const OUString& rJobName,
                                              const OUString& rAppName,
                                              sal_uInt32 nCopies,
                                              bool bCollate,
                                              bool bDirect,
                                              ImplJobSetup* pSetupData ) = 0;

    // implement for pull model print systems only,
    // default implementations (see salvtables.cxx) just returns false
    virtual bool                    StartJob( const OUString* pFileName,
                                              const OUString& rJobName,
                                              const OUString& rAppName,
                                              ImplJobSetup* pSetupData,
                                              vcl::PrinterController& rController );

    virtual bool                    EndJob() = 0;
    virtual SalGraphics*            StartPage( ImplJobSetup* pSetupData, bool bNewJobData ) = 0;
    virtual void                    EndPage() = 0;
    virtual sal_uLong               GetErrorCode() = 0;

};

#endif // INCLUDED_VCL_INC_SALPRN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
