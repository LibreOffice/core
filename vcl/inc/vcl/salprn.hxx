/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salprn.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:18:09 $
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

#ifndef _SV_SALPRN_HXX
#define _SV_SALPRN_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_PRNTYPES_HXX
#include <vcl/prntypes.hxx>
#endif

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
