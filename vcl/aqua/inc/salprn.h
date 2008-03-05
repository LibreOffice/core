/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salprn.h,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:58:38 $
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

#ifndef _SV_SALPRN_H
#define _SV_SALPRN_H

#include "vcl/sv.h"
#include "aquavcltypes.h"
#include "vcl/salprn.hxx"

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

    public:
    AquaSalInfoPrinter( const SalPrinterQueueInfo& pInfo );
    virtual ~AquaSalInfoPrinter();

    void                        SetupPrinterGraphics( CGContextRef i_xContext ) const;

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* i_pGraphics );
    virtual BOOL                Setup( SalFrame* i_pFrame, ImplJobSetup* i_pSetupData );
    virtual BOOL                SetPrinterData( ImplJobSetup* pSetupData );
    virtual BOOL                SetData( ULONG i_nFlags, ImplJobSetup* i_pSetupData );
    virtual void                GetPageInfo( const ImplJobSetup* i_pSetupData,
                                             long& o_rOutWidth, long& o_rOutHeight,
                                             long& o_rPageOffX, long& o_rPageOffY,
                                             long& o_rPageWidth, long& o_rPageHeight );
    virtual ULONG               GetCapabilities( const ImplJobSetup* i_pSetupData, USHORT i_nType );
    virtual ULONG               GetPaperBinCount( const ImplJobSetup* i_pSetupData );
    virtual String              GetPaperBinName( const ImplJobSetup* i_pSetupData, ULONG i_nPaperBin );
    virtual void                InitPaperFormats( const ImplJobSetup* i_pSetupData );
    virtual int                 GetLandscapeAngle( const ImplJobSetup* i_pSetupData );
    virtual DuplexMode          GetDuplexMode( const ImplJobSetup* i_pSetupData );


    // the artificial separation between InfoPrinter and Printer
    // is not really useful for us
    // so let's make AquaSalPrinter just a forwarder to AquaSalInfoPrinter
    // and concentrate the real work in one class
    // implement pull model print system
    BOOL                        StartJob( const String* pFileName,
                                          const String& rAppName,
                                          ImplJobSetup* pSetupData,
                                          ImplQPrinter* pQPrinter,
                                          bool bIsQuickJob );
    BOOL                        EndJob();
    BOOL                        AbortJob();
    SalGraphics*                StartPage( ImplJobSetup* i_pSetupData, BOOL i_bNewJobData );
    BOOL                        EndPage();
    ULONG                       GetErrorCode() const;

    NSPrintInfo* getPrintInfo() const { return mpPrintInfo; }
    void setStartPageOffset( int nOffsetX, int nOffsetY ) { mnStartPageOffsetX = nOffsetX; mnStartPageOffsetY = nOffsetY; }

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

    virtual BOOL                    StartJob( const XubString* i_pFileName,
                                              const XubString& i_rJobName,
                                              const XubString& i_rAppName,
                                              ULONG i_nCopies, BOOL i_bCollate,
                                              ImplJobSetup* i_pSetupData );
    // implement pull model print system
    virtual BOOL                    StartJob( const String* pFileName,
                                              const String& rAppName,
                                              ImplJobSetup* pSetupData,
                                              ImplQPrinter* pQPrinter );

    virtual BOOL                    EndJob();
    virtual BOOL                    AbortJob();
    virtual SalGraphics*            StartPage( ImplJobSetup* i_pSetupData, BOOL i_bNewJobData );
    virtual BOOL                    EndPage();
    virtual ULONG                   GetErrorCode();

    private:
    AquaSalPrinter( const AquaSalPrinter& );
    AquaSalPrinter& operator=(const AquaSalPrinter&);
};

const double fPtTo100thMM = 35.27777778;

inline int PtTo10Mu( double nPoints ) { return (int)(((nPoints)*fPtTo100thMM)+0.5); }

inline double TenMuToPt( double nUnits ) { return (((nUnits)/fPtTo100thMM)+0.5); }



#endif // _SV_SALPRN_H
