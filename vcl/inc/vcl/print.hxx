/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: print.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _SV_PRINT_HXX
#define _SV_PRINT_HXX

#include <tools/errcode.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>
#include <vcl/prntypes.hxx>
#include <vcl/jobset.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/stream.hxx>

struct SalPrinterInfoQueue;
class SalInfoPrinter;
struct SalPrinterQueueInfo;
class SalPrinter;
class VirtualDevice;
class Window;
class ImplQPrinter;
class RmPrinter;
struct ImplPrivatePrinterData;

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

// -----------------
// - Printer-Types -
// -----------------

#define PAGEQUEUE_ALLPAGES   0xFFFF

enum PrinterSupport { SUPPORT_SET_ORIENTATION, SUPPORT_SET_PAPERBIN,
                      SUPPORT_SET_PAPERSIZE, SUPPORT_SET_PAPER,
                      SUPPORT_COPY, SUPPORT_COLLATECOPY,
                      SUPPORT_SETUPDIALOG, SUPPORT_FAX, SUPPORT_PDF };

// ---------------
// - PrinterPage -
// ---------------

class VCL_DLLPUBLIC PrinterPage
{
    GDIMetaFile*    mpMtf;
    JobSetup        maJobSetup;
    UINT16          mbNewJobSetup;

public:

    PrinterPage() : mpMtf( new GDIMetaFile() ) {}
    PrinterPage( GDIMetaFile* pMtf, BOOL bNewJobSetup, const JobSetup& rSetup ) :
           mpMtf( pMtf ), maJobSetup( rSetup ), mbNewJobSetup( bNewJobSetup ) {}
    ~PrinterPage() { delete mpMtf; }

    GDIMetaFile*    GetGDIMetaFile() const { return mpMtf; }
    const JobSetup& GetJobSetup() const { return maJobSetup; }
    BOOL            IsNewJobSetup() const { return (mbNewJobSetup != 0); }

    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const PrinterPage& rPage )
    { rOStm << rPage.mbNewJobSetup; rOStm << rPage.maJobSetup; rPage.mpMtf->Write( rOStm ); return rOStm; }
    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, PrinterPage& rPage )
    { return rIStm >> rPage.mbNewJobSetup >> rPage.maJobSetup >> *rPage.mpMtf; }
};


// -------------
// - QueueInfo -
// -------------

class VCL_DLLPUBLIC QueueInfo
{
    friend class Printer;

private:
    XubString                   maPrinterName;
    XubString                   maDriver;
    XubString                   maLocation;
    XubString                   maComment;
    sal_uInt32                  mnStatus;
    sal_uInt32                  mnJobs;

public:
                                QueueInfo();
                                QueueInfo( const QueueInfo& rInfo );
                                ~QueueInfo();

    const XubString&            GetPrinterName() const { return maPrinterName; }
    const XubString&            GetDriver() const { return maDriver; }
    const XubString&            GetLocation() const { return maLocation; }
    const XubString&            GetComment() const { return maComment; }
    sal_uInt32                  GetStatus() const { return mnStatus; }
    sal_uInt32                  GetJobs() const { return mnJobs; }

    bool operator==( const QueueInfo& rInfo ) const;

    friend VCL_DLLPUBLIC SvStream&          operator>>( SvStream& rIStream, QueueInfo& rInfo );
    friend VCL_DLLPUBLIC SvStream&          operator<<( SvStream& rOStream, const QueueInfo& rInfo );
};

// ------------------
// - PrinterOptions -
// ------------------

enum PrinterTransparencyMode
{
    PRINTER_TRANSPARENCY_AUTO = 0,
    PRINTER_TRANSPARENCY_NONE = 1
};

// -----------------------------------------------------------------------------

enum PrinterGradientMode
{
    PRINTER_GRADIENT_STRIPES = 0,
    PRINTER_GRADIENT_COLOR = 1
};

// -----------------------------------------------------------------------------

enum PrinterBitmapMode
{
    PRINTER_BITMAP_OPTIMAL = 0,
    PRINTER_BITMAP_NORMAL = 1,
    PRINTER_BITMAP_RESOLUTION = 2
};

// -----------------------------------------------------------------------------

class VCL_DLLPUBLIC PrinterOptions
{
private:

    BOOL                        mbReduceTransparency;
    PrinterTransparencyMode     meReducedTransparencyMode;
    BOOL                        mbReduceGradients;
    PrinterGradientMode         meReducedGradientsMode;
    USHORT                      mnReducedGradientStepCount;
    BOOL                        mbReduceBitmaps;
    PrinterBitmapMode           meReducedBitmapMode;
    USHORT                      mnReducedBitmapResolution;
    BOOL                        mbReducedBitmapsIncludeTransparency;
    BOOL                        mbConvertToGreyscales;

public:

                                PrinterOptions();
                                ~PrinterOptions();

    BOOL                        IsReduceTransparency() const { return mbReduceTransparency; }
    void                        SetReduceTransparency( BOOL bSet ) { mbReduceTransparency = bSet; }

    PrinterTransparencyMode     GetReducedTransparencyMode() const { return meReducedTransparencyMode; }
    void                        SetReducedTransparencyMode( PrinterTransparencyMode eMode ) { meReducedTransparencyMode = eMode; }

    BOOL                        IsReduceGradients() const { return mbReduceGradients; }
    void                        SetReduceGradients( BOOL bSet ) { mbReduceGradients = bSet; }

    PrinterGradientMode         GetReducedGradientMode() const { return meReducedGradientsMode; }
    void                        SetReducedGradientMode( PrinterGradientMode eMode ) { meReducedGradientsMode = eMode; }

    USHORT                      GetReducedGradientStepCount() const { return mnReducedGradientStepCount; }
    void                        SetReducedGradientStepCount( USHORT nStepCount ) { mnReducedGradientStepCount = nStepCount; }

    BOOL                        IsReduceBitmaps() const { return mbReduceBitmaps; }
    void                        SetReduceBitmaps( BOOL bSet ) { mbReduceBitmaps = bSet; }

    PrinterBitmapMode           GetReducedBitmapMode() const { return meReducedBitmapMode; }
    void                        SetReducedBitmapMode( PrinterBitmapMode eMode ) { meReducedBitmapMode = eMode; }

    USHORT                      GetReducedBitmapResolution() const { return mnReducedBitmapResolution; }
    void                        SetReducedBitmapResolution( USHORT nResolution ) { mnReducedBitmapResolution = nResolution; }

    BOOL                        IsReducedBitmapIncludesTransparency() const { return mbReducedBitmapsIncludeTransparency; }
    void                        SetReducedBitmapIncludesTransparency( BOOL bSet ) { mbReducedBitmapsIncludeTransparency = bSet; }

    BOOL                        IsConvertToGreyscales() const { return mbConvertToGreyscales; }
    void                        SetConvertToGreyscales( BOOL bSet ) { mbConvertToGreyscales = bSet; }
};

// -----------
// - Printer -
// -----------

class VCL_DLLPUBLIC Printer : public OutputDevice
{
    friend class OutputDevice;
    friend class ImplQPrinter;

private:
    ImplPrivatePrinterData*     mpPrinterData;
    SalInfoPrinter*             mpInfoPrinter;
    SalPrinter*                 mpPrinter;
    Printer*                    mpJobPrinter;
    SalGraphics*                mpJobGraphics;
    Printer*                    mpPrev;
    Printer*                    mpNext;
    VirtualDevice*              mpDisplayDev;
    ImplQPrinter*               mpQPrinter;
    GDIMetaFile*                mpQMtf;
    PrinterOptions*             mpPrinterOptions;
    XubString                   maPrinterName;
    XubString                   maDriver;
    XubString                   maPrintFile;
    XubString                   maJobName;
    JobSetup                    maJobSetup;
    Point                       maPageOffset;
    Size                        maPaperSize;
    ULONG                       mnError;
    USHORT                      mnCurPage;
    USHORT                      mnCurPrintPage;
    USHORT                      mnPageQueueSize;
    USHORT                      mnCopyCount;
    BOOL                        mbDefPrinter;
    BOOL                        mbPrinting;
    BOOL                        mbJobActive;
    BOOL                        mbCollateCopy;
    BOOL                        mbPrintFile;
    BOOL                        mbInPrintPage;
    BOOL                        mbNewJobSetup;
    BOOL                        mbIsQueuePrinter;
    BOOL                        mbUserSetupCompleted;
    BOOL                        mbUserSetupResult;
    Link                        maErrorHdl;
    Link                        maStartPrintHdl;
    Link                        maEndPrintHdl;
    Link                        maPrintPageHdl;

    SAL_DLLPRIVATE void         ImplInitData();
    SAL_DLLPRIVATE void         ImplInit( SalPrinterQueueInfo* pInfo );
    SAL_DLLPRIVATE void         ImplInitDisplay( const Window* pWindow );
    SAL_DLLPRIVATE static SalPrinterQueueInfo* ImplGetQueueInfo( const XubString& rPrinterName,
                                                  const XubString* pDriver );
    SAL_DLLPRIVATE void         ImplUpdatePageData();
    SAL_DLLPRIVATE void         ImplUpdateFontList();
    SAL_DLLPRIVATE void         ImplFindPaperFormatForUserSize( JobSetup& );
    DECL_DLLPRIVATE_LINK(       ImplDestroyPrinterAsync, void* );
public:
    SAL_DLLPRIVATE void         ImplEndPrint();
    SAL_DLLPRIVATE void         ImplUpdateQuickStatus();
private:
    SAL_DLLPRIVATE              Printer( const Printer& rPrinter );
    SAL_DLLPRIVATE Printer&     operator =( const Printer& rPrinter );

#ifdef _SPOOLPRINTER_EXT
public:
    void                        GetPreparedMetaFile( const GDIMetaFile& rInMtf, GDIMetaFile& rOutMtf,
                                                     long nMaxBmpDPIX, long nMaxBmpDPIY );
    /** Retrieve downsampled and cropped bitmap

        @attention This method ignores negative rDstSz values, thus
        mirroring must happen outside this method (e.g. in DrawBitmap)
     */
    Bitmap                      GetPreparedBitmap( const Size& rDstSz,
                                                   const Point& rSrcPt, const Size& rSrcSz,
                                                   const Bitmap& rBmp, long nMaxBmpDPIX, long nMaxBmpDPIY );
    /** Retrieve downsampled and cropped bitmapEx

        @attention This method ignores negative rDstSz values, thus
        mirroring must happen outside this method (e.g. in DrawBitmapEx)
     */
    BitmapEx                    GetPreparedBitmapEx( const Size& rDstSz,
                                                     const Point& rSrcPt, const Size& rSrcSz,
                                                     const BitmapEx& rBmpEx, long nMaxBmpDPIX, long nMaxBmpDPIY );
    void                        DrawGradientEx( OutputDevice* pOut, const Rectangle& rRect, const Gradient& rGradient );
    void                        DrawGradientEx( OutputDevice* pOut, const PolyPolygon& rPolyPoly, const Gradient& rGradient );

#endif // _SPOOLPRINTER_EXT

protected:

    void                        SetSelfAsQueuePrinter( BOOL bQueuePrinter ) { mbIsQueuePrinter = bQueuePrinter; }
    BOOL                        IsQueuePrinter() const { return mbIsQueuePrinter; }

public:
                                Printer();
                                Printer( const Window* pWindow );
                                Printer( const JobSetup& rJobSetup );
                                Printer( const QueueInfo& rQueueInfo );
                                Printer( const XubString& rPrinterName );
    virtual                     ~Printer();

    static const std::vector< rtl::OUString >& GetPrinterQueues();
    static const QueueInfo*     GetQueueInfo( const String& rPrinterName, bool bStatusUpdate );
    static XubString            GetDefaultPrinterName();

    virtual void                Error();
    virtual void                StartPrint();
    virtual void                EndPrint();
    virtual void                PrintPage();

    const XubString&            GetName() const             { return maPrinterName; }
    const XubString&            GetDriverName() const       { return maDriver; }
    BOOL                        IsDefPrinter() const        { return mbDefPrinter; }
    BOOL                        IsDisplayPrinter() const    { return mpDisplayDev != NULL; }
    BOOL                        IsValid() const             { return !IsDisplayPrinter(); }

    ULONG                       GetCapabilities( USHORT nType ) const;
    BOOL                        HasSupport( PrinterSupport eFeature ) const;

    BOOL                        SetJobSetup( const JobSetup& rSetup );
    const JobSetup&             GetJobSetup() const { return maJobSetup; }
    String                      GetJobValue( const String& rKey ) const { return maJobSetup.GetValue( rKey ); }
    void                        SetJobValue( const String& rKey, const String& rValue ) { maJobSetup.SetValue( rKey, rValue ); }

    BOOL                        Setup( Window* pWindow = NULL );
    BOOL                        SetPrinterProps( const Printer* pPrinter );

    void                        SetPrinterOptions( const PrinterOptions& rOptions ) { *mpPrinterOptions = rOptions; }
    const PrinterOptions&       GetPrinterOptions() const { return( *mpPrinterOptions ); }

    BOOL                        SetOrientation( Orientation eOrient );
    Orientation                 GetOrientation() const;
    DuplexMode                  GetDuplexMode() const;
    // returns the angle that a landscape page will be turned counterclockwise
    // wrt to portrait. The return value may be only valid for
    // the current paper
    int                         GetLandscapeAngle() const;
    BOOL                        SetPaperBin( USHORT nPaperBin );
    USHORT                      GetPaperBin() const;
    BOOL                        SetPaper( Paper ePaper );
    BOOL                        SetPaperSizeUser( const Size& rSize );
    Paper                       GetPaper() const;

    // returns number of available paper formats
    int                         GetPaperInfoCount() const;
    // returns info about paper format nPaper
    const vcl::PaperInfo&       GetPaperInfo( int nPaper ) const;
    // sets current paper to format contained in rInfo
    BOOL                        SetPaperFromInfo( const vcl::PaperInfo& rInfo );
    // gets info about paper fromat best matching current paper
    const vcl::PaperInfo&       GetCurrentPaperInfo() const;

    USHORT                      GetPaperBinCount() const;
    XubString                   GetPaperBinName( USHORT nPaperBin ) const;

    const Size&                 GetPaperSizePixel() const { return maPaperSize; }
    Size                        GetPaperSize() const { return PixelToLogic( maPaperSize ); }
    const Point&                GetPageOffsetPixel() const { return maPageOffset; }
    Point                       GetPageOffset() const { return PixelToLogic( maPageOffset ); }

    BOOL                        SetCopyCount( USHORT nCopy, BOOL bCollate = FALSE );
    USHORT                      GetCopyCount() const { return mnCopyCount; }
    BOOL                        IsCollateCopy() const { return mbCollateCopy; }

    USHORT                      GetCurPrintPage() const { return mnCurPrintPage; }
    BOOL                        IsPrinting() const { return mbPrinting; }

    void                        SetPrintFile( const XubString& rFileName ) { maPrintFile = rFileName; }
    const XubString&            GetPrintFile() const { return maPrintFile; }
    void                        EnablePrintFile( BOOL bEnable ) { mbPrintFile = bEnable; }
    BOOL                        IsPrintFileEnabled() const { return mbPrintFile; }
    BOOL                        StartJob( const XubString& rJobName );
    BOOL                        EndJob();
    BOOL                        AbortJob();
    const XubString&            GetCurJobName() const { return maJobName; }
    USHORT                      GetCurPage() const { return mnCurPage; }
    BOOL                        IsJobActive() const { return mbJobActive; }
    BOOL                        StartPage();
    BOOL                        EndPage();

    void                        SetPageQueueSize( USHORT nPages ) { mnPageQueueSize = nPages; }
    USHORT                      GetPageQueueSize() const { return mnPageQueueSize; }

    ULONG                       GetError() const { return ERRCODE_TOERROR(mnError); }
    ULONG                       GetErrorCode() const { return mnError; }

    void                        SetErrorHdl( const Link& rLink ) { maErrorHdl = rLink; }
    const Link&                 GetErrorHdl() const { return maErrorHdl; }
    void                        SetStartPrintHdl( const Link& rLink ) { maStartPrintHdl = rLink; }
    const Link&                 GetStartPrintHdl() const { return maStartPrintHdl; }
    void                        SetEndPrintHdl( const Link& rLink ) { maEndPrintHdl = rLink; }
    const Link&                 GetEndPrintHdl() const   { return maEndPrintHdl;   }
    void                        SetPrintPageHdl( const Link& rLink ) { maPrintPageHdl = rLink; }
    const Link&                 GetPrintPageHdl() const  { return maPrintPageHdl;  }

    void                        Compat_OldPrinterMetrics( bool bSet );

    /** Notify that the next StartJob belongs to a UI less "direct print" job
    *
    *   deprecated: the canonical way to notify a UI less job is to set the
    *   JobSetup value "IsQuickJob" to "true". If set at all, the "IsQuickJob" value
    *   on JobSetup will be preferred. However if no "IsQuickJob" value is set,
    *   setting SetNextJobIsQuick will cause the following StartJob to set this value
    *   to "true" in the current JobSetup.
    */
    void                        SetNextJobIsQuick();

    /** checks the printer list and updates it necessary
    *
    *   sends a DataChanged event of type DATACHANGED_PRINTER
    *   if the printer list changed
    */
    static void updatePrinters();
};

#endif  // _SV_PRINT_HXX
