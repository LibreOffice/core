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

#ifndef INCLUDED_VCL_INC_OSX_SALPRN_H
#define INCLUDED_VCL_INC_OSX_SALPRN_H

#include "osx/osxvcltypes.h"

#include "salprn.hxx"

#include <memory>

class AquaSalGraphics;

class AquaSalInfoPrinter : public SalInfoPrinter
{
    /// Printer graphics
    AquaSalGraphics*        mpGraphics;
    /// is Graphics used
    bool                    mbGraphics;
    /// job active ?
    bool                    mbJob;

    /// cocoa printer object
    NSPrinter*              mpPrinter;
    /// cocoa print info object
    NSPrintInfo*            mpPrintInfo;

    /// FIXME: get real printer context for infoprinter if possible
    /// fake context for info printer
    /// graphics context for Quartz 2D
    CGContextRef                            mrContext;
    /// memory for graphics bitmap context for querying metrics
    std::shared_ptr<sal_uInt8> mpContextMemory;

    // since changes to NSPrintInfo during a job are ignored
    // we have to care for some settings ourselves
    // currently we do this for orientation;
    // really needed however is a solution for paper formats
    Orientation               mePageOrientation;

    int                       mnStartPageOffsetX;
    int                       mnStartPageOffsetY;
    sal_Int32                 mnCurPageRangeStart;
    sal_Int32                 mnCurPageRangeCount;

    public:
    AquaSalInfoPrinter( const SalPrinterQueueInfo& pInfo );
    virtual ~AquaSalInfoPrinter() override;

    void                        SetupPrinterGraphics( CGContextRef i_xContext ) const;

    virtual SalGraphics*        AcquireGraphics() override;
    virtual void                ReleaseGraphics( SalGraphics* i_pGraphics ) override;
    virtual bool                Setup( SalFrame* i_pFrame, ImplJobSetup* i_pSetupData ) override;
    virtual bool                SetPrinterData( ImplJobSetup* pSetupData ) override;
    virtual bool                SetData( JobSetFlags i_nFlags, ImplJobSetup* i_pSetupData ) override;
    virtual void                GetPageInfo( const ImplJobSetup* i_pSetupData,
                                             long& o_rOutWidth, long& o_rOutHeight,
                                             long& o_rPageOffX, long& o_rPageOffY,
                                             long& o_rPageWidth, long& o_rPageHeight ) override;
    virtual sal_uInt32          GetCapabilities( const ImplJobSetup* i_pSetupData, PrinterCapType i_nType ) override;
    virtual sal_uInt16          GetPaperBinCount( const ImplJobSetup* i_pSetupData ) override;
    virtual OUString            GetPaperBinName( const ImplJobSetup* i_pSetupData, sal_uInt16 i_nPaperBin ) override;
    virtual void                InitPaperFormats( const ImplJobSetup* i_pSetupData ) override;
    virtual int                 GetLandscapeAngle( const ImplJobSetup* i_pSetupData ) override;

    // the artificial separation between InfoPrinter and Printer
    // is not really useful for us
    // so let's make AquaSalPrinter just a forwarder to AquaSalInfoPrinter
    // and concentrate the real work in one class
    // implement pull model print system
    bool                        StartJob( const OUString* i_pFileName,
                                          const OUString& rJobName,
                                          const OUString& i_rAppName,
                                          ImplJobSetup* i_pSetupData,
                                          vcl::PrinterController& i_rController );
    bool                        EndJob();
    bool                        AbortJob();
    SalGraphics*                StartPage( ImplJobSetup* i_pSetupData, bool i_bNewJobData );
    bool                        EndPage();
    static sal_uLong            GetErrorCode();

    NSPrintInfo* getPrintInfo() const { return mpPrintInfo; }
    void setStartPageOffset( int nOffsetX, int nOffsetY ) { mnStartPageOffsetX = nOffsetX; mnStartPageOffsetY = nOffsetY; }
    sal_Int32 getCurPageRangeStart() const { return mnCurPageRangeStart; }
    sal_Int32 getCurPageRangeCount() const { return mnCurPageRangeCount; }

    // match width/height against known paper formats, possibly switching orientation
    const PaperInfo* matchPaper( long i_nWidth, long i_nHeight, Orientation& o_rOrientation ) const;
    void setPaperSize( long i_nWidth, long i_nHeight, Orientation i_eSetOrientation );

    private:
    AquaSalInfoPrinter( const AquaSalInfoPrinter& ) = delete;
    AquaSalInfoPrinter& operator=(const AquaSalInfoPrinter&) = delete;
};


class AquaSalPrinter : public SalPrinter
{
    AquaSalInfoPrinter*         mpInfoPrinter;          // pointer to the compatible InfoPrinter
    public:
    AquaSalPrinter( AquaSalInfoPrinter* i_pInfoPrinter );
    virtual ~AquaSalPrinter() override;

    virtual bool                    StartJob( const OUString* i_pFileName,
                                              const OUString& i_rJobName,
                                              const OUString& i_rAppName,
                                              sal_uInt32 i_nCopies,
                                              bool i_bCollate,
                                              bool i_bDirect,
                                              ImplJobSetup* i_pSetupData ) override;
    // implement pull model print system
    virtual bool                    StartJob( const OUString* i_pFileName,
                                              const OUString& rJobName,
                                              const OUString& i_rAppName,
                                              ImplJobSetup* i_pSetupData,
                                              vcl::PrinterController& i_rListener ) override;

    virtual bool                    EndJob() override;
    virtual SalGraphics*            StartPage( ImplJobSetup* i_pSetupData, bool i_bNewJobData ) override;
    virtual void                    EndPage() override;
    virtual sal_uLong               GetErrorCode() override;

    private:
    AquaSalPrinter( const AquaSalPrinter& ) = delete;
    AquaSalPrinter& operator=(const AquaSalPrinter&) = delete;
};

const double fPtTo100thMM = 35.27777778;

inline int PtTo10Mu( double nPoints ) { return (int)(((nPoints)*fPtTo100thMM)+0.5); }

inline double TenMuToPt( double nUnits ) { return floor(((nUnits)/fPtTo100thMM)+0.5); }

#endif // INCLUDED_VCL_INC_OSX_SALPRN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
