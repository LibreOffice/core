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

#include "aqua/aquavcltypes.h"

#include "salprn.hxx"

#include <boost/shared_array.hpp>


// ---------------------
// - AquaSalInfoPrinter -
// ---------------------

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
    boost::shared_array< sal_uInt8 >        maContextMemory;

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
    virtual ~AquaSalInfoPrinter();

    void                        SetupPrinterGraphics( CGContextRef i_xContext ) const;

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* i_pGraphics );
    virtual sal_Bool                Setup( SalFrame* i_pFrame, ImplJobSetup* i_pSetupData );
    virtual sal_Bool                SetPrinterData( ImplJobSetup* pSetupData );
    virtual sal_Bool                SetData( sal_uLong i_nFlags, ImplJobSetup* i_pSetupData );
    virtual void                GetPageInfo( const ImplJobSetup* i_pSetupData,
                                             long& o_rOutWidth, long& o_rOutHeight,
                                             long& o_rPageOffX, long& o_rPageOffY,
                                             long& o_rPageWidth, long& o_rPageHeight );
    virtual sal_uLong               GetCapabilities( const ImplJobSetup* i_pSetupData, sal_uInt16 i_nType );
    virtual sal_uLong               GetPaperBinCount( const ImplJobSetup* i_pSetupData );
    virtual String              GetPaperBinName( const ImplJobSetup* i_pSetupData, sal_uLong i_nPaperBin );
    virtual void                InitPaperFormats( const ImplJobSetup* i_pSetupData );
    virtual int                 GetLandscapeAngle( const ImplJobSetup* i_pSetupData );

    // the artificial separation between InfoPrinter and Printer
    // is not really useful for us
    // so let's make AquaSalPrinter just a forwarder to AquaSalInfoPrinter
    // and concentrate the real work in one class
    // implement pull model print system
    sal_Bool                        StartJob( const String* i_pFileName,
                                          const String& rJobName,
                                          const String& i_rAppName,
                                          ImplJobSetup* i_pSetupData,
                                          vcl::PrinterController& i_rController );
    sal_Bool                        EndJob();
    sal_Bool                        AbortJob();
    SalGraphics*                StartPage( ImplJobSetup* i_pSetupData, sal_Bool i_bNewJobData );
    sal_Bool                        EndPage();
    sal_uLong                       GetErrorCode() const;

    NSPrintInfo* getPrintInfo() const { return mpPrintInfo; }
    void setStartPageOffset( int nOffsetX, int nOffsetY ) { mnStartPageOffsetX = nOffsetX; mnStartPageOffsetY = nOffsetY; }
    sal_Int32 getCurPageRangeStart() const { return mnCurPageRangeStart; }
    sal_Int32 getCurPageRangeCount() const { return mnCurPageRangeCount; }

    // match width/height against known paper formats, possibly switching orientation
    const PaperInfo* matchPaper( long i_nWidth, long i_nHeight, Orientation& o_rOrientation ) const;
    void setPaperSize( long i_nWidth, long i_nHeight, Orientation i_eSetOrientation );

    private:
    AquaSalInfoPrinter( const AquaSalInfoPrinter& );
    AquaSalInfoPrinter& operator=(const AquaSalInfoPrinter&);
};

// -----------------
// - AquaSalPrinter -
// -----------------

class AquaSalPrinter : public SalPrinter
{
    AquaSalInfoPrinter*         mpInfoPrinter;          // pointer to the compatible InfoPrinter
    public:
    AquaSalPrinter( AquaSalInfoPrinter* i_pInfoPrinter );
    virtual ~AquaSalPrinter();

    virtual sal_Bool                    StartJob( const XubString* i_pFileName,
                                              const XubString& i_rJobName,
                                              const XubString& i_rAppName,
                                              sal_uLong i_nCopies,
                                              bool i_bCollate,
                                              bool i_bDirect,
                                              ImplJobSetup* i_pSetupData );
    // implement pull model print system
    virtual sal_Bool                    StartJob( const String* i_pFileName,
                                              const String& rJobName,
                                              const String& i_rAppName,
                                              ImplJobSetup* i_pSetupData,
                                              vcl::PrinterController& i_rListener );

    virtual sal_Bool                    EndJob();
    virtual sal_Bool                    AbortJob();
    virtual SalGraphics*            StartPage( ImplJobSetup* i_pSetupData, sal_Bool i_bNewJobData );
    virtual sal_Bool                    EndPage();
    virtual sal_uLong                   GetErrorCode();

    private:
    AquaSalPrinter( const AquaSalPrinter& );
    AquaSalPrinter& operator=(const AquaSalPrinter&);
};

const double fPtTo100thMM = 35.27777778;

inline int PtTo10Mu( double nPoints ) { return (int)(((nPoints)*fPtTo100thMM)+0.5); }

inline double TenMuToPt( double nUnits ) { return floor(((nUnits)/fPtTo100thMM)+0.5); }



#endif // _SV_SALPRN_H
