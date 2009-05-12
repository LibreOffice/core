/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salprn.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SV_SALPRN_HXX
#define _SV_SALPRN_HXX

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/prntypes.hxx>

#include <vector>

class SalGraphics;
class SalFrame;
struct ImplJobSetup;
class ImplQPrinter;

// -----------------------
// - SalPrinterQueueInfo -
// -----------------------

struct VCL_DLLPUBLIC SalPrinterQueueInfo
{
    XubString               maPrinterName;
    XubString               maDriver;
    XubString               maLocation;
    XubString               maComment;
    ULONG                   mnStatus;
    ULONG                   mnJobs;
    void*                   mpSysData;

                            SalPrinterQueueInfo();
                            ~SalPrinterQueueInfo();
};

// ------------------
// - SalInfoPrinter -
// ------------------

class VCL_DLLPUBLIC SalInfoPrinter
{
public:
    std::vector< vcl::PaperInfo  >      m_aPaperFormats;    // all printer supported formats
    bool                                m_bPapersInit;      // set to true after InitPaperFormats
    bool                                m_bCompatMetrics;

    SalInfoPrinter() : m_bPapersInit( false ), m_bCompatMetrics( false ) {}
    virtual ~SalInfoPrinter();

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*            GetGraphics() = 0;
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics ) = 0;

    virtual BOOL                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData ) = 0;
    // This function set the driver data and
    // set the new indepen data in pSetupData
    virtual BOOL                    SetPrinterData( ImplJobSetup* pSetupData ) = 0;
    // This function merged the indepen driver data
    // and set the new indepen data in pSetupData
    // Only the data must changed, where the bit
    // in nFlags is set
    virtual BOOL                    SetData( ULONG nFlags, ImplJobSetup* pSetupData ) = 0;

    virtual void                    GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight ) = 0;
    virtual ULONG                   GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType ) = 0;
    virtual ULONG                   GetPaperBinCount( const ImplJobSetup* pSetupData ) = 0;
    virtual String                  GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin ) = 0;
    // fills m_aPaperFormats and sets m_bPapersInit to true
    virtual void                    InitPaperFormats( const ImplJobSetup* pSetupData ) = 0;
    // returns angle that a landscape page will be turned counterclockwise wrt to portrait
    virtual int                 GetLandscapeAngle( const ImplJobSetup* pSetupData ) = 0;
    virtual DuplexMode          GetDuplexMode( const ImplJobSetup* pSetupData ) = 0;
};

// --------------
// - SalPrinter -
// --------------

class VCL_DLLPUBLIC SalPrinter
{
public:                     // public for Sal Implementation
    SalPrinter() {}
    virtual ~SalPrinter();

    virtual BOOL                    StartJob( const XubString* pFileName,
                                              const XubString& rJobName,
                                              const XubString& rAppName,
                                              ULONG nCopies, BOOL bCollate,
                                              ImplJobSetup* pSetupData ) = 0;

    // implement for pull model print systems only,
    // default implementations (see salvtables.cxx) just returns FALSE
    virtual BOOL                    StartJob( const String* pFileName,
                                              const String& rAppName,
                                              ImplJobSetup* pSetupData,
                                              ImplQPrinter* pQPrinter );

    virtual BOOL                    EndJob() = 0;
    virtual BOOL                    AbortJob() = 0;
    virtual SalGraphics*            StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData ) = 0;
    virtual BOOL                    EndPage() = 0;
    virtual ULONG                   GetErrorCode() = 0;

};

#endif // _SV_SALPRN_HXX
