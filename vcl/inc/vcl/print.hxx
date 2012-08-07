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

#ifndef _SV_PRINT_HXX
#define _SV_PRINT_HXX

#include <rtl/ustring.hxx>

#include "tools/errcode.hxx"
#include "tools/solar.h"
#include "vcl/dllapi.h"
#include "vcl/outdev.hxx"
#include "vcl/prntypes.hxx"
#include "vcl/jobset.hxx"
#include "vcl/gdimtf.hxx"
#include "tools/stream.hxx"
#include "tools/multisel.hxx"

#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/view/PrintableState.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <set>

class SalInfoPrinter;
struct SalPrinterQueueInfo;
class SalPrinter;
class VirtualDevice;
class Window;

namespace vcl {
    class PrinterController;
    class PrintDialog;
}

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
    sal_uInt16          mbNewJobSetup;

public:

    PrinterPage() : mpMtf( new GDIMetaFile() ) {}
    PrinterPage( GDIMetaFile* pMtf, sal_Bool bNewJobSetup, const JobSetup& rSetup ) :
           mpMtf( pMtf ), maJobSetup( rSetup ), mbNewJobSetup( bNewJobSetup ) {}
    ~PrinterPage() { delete mpMtf; }

    GDIMetaFile*    GetGDIMetaFile() const { return mpMtf; }
    const JobSetup& GetJobSetup() const { return maJobSetup; }
    sal_Bool            IsNewJobSetup() const { return (mbNewJobSetup != 0); }

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
    rtl::OUString                   maPrinterName;
    rtl::OUString                   maDriver;
    rtl::OUString                   maLocation;
    rtl::OUString                   maComment;
    sal_uInt32                  mnStatus;
    sal_uInt32                  mnJobs;

public:
                                QueueInfo();
                                QueueInfo( const QueueInfo& rInfo );
                                ~QueueInfo();

    const rtl::OUString&            GetPrinterName() const { return maPrinterName; }
    const rtl::OUString&            GetDriver() const { return maDriver; }
    const rtl::OUString&            GetLocation() const { return maLocation; }
    const rtl::OUString&            GetComment() const { return maComment; }
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

    sal_Bool                        mbReduceTransparency;
    PrinterTransparencyMode     meReducedTransparencyMode;
    sal_Bool                        mbReduceGradients;
    PrinterGradientMode         meReducedGradientsMode;
    sal_uInt16                      mnReducedGradientStepCount;
    sal_Bool                        mbReduceBitmaps;
    PrinterBitmapMode           meReducedBitmapMode;
    sal_uInt16                      mnReducedBitmapResolution;
    sal_Bool                        mbReducedBitmapsIncludeTransparency;
    sal_Bool                        mbConvertToGreyscales;

public:

                                PrinterOptions();
                                ~PrinterOptions();

    sal_Bool                        IsReduceTransparency() const { return mbReduceTransparency; }
    void                        SetReduceTransparency( sal_Bool bSet ) { mbReduceTransparency = bSet; }

    PrinterTransparencyMode     GetReducedTransparencyMode() const { return meReducedTransparencyMode; }
    void                        SetReducedTransparencyMode( PrinterTransparencyMode eMode ) { meReducedTransparencyMode = eMode; }

    sal_Bool                        IsReduceGradients() const { return mbReduceGradients; }
    void                        SetReduceGradients( sal_Bool bSet ) { mbReduceGradients = bSet; }

    PrinterGradientMode         GetReducedGradientMode() const { return meReducedGradientsMode; }
    void                        SetReducedGradientMode( PrinterGradientMode eMode ) { meReducedGradientsMode = eMode; }

    sal_uInt16                      GetReducedGradientStepCount() const { return mnReducedGradientStepCount; }
    void                        SetReducedGradientStepCount( sal_uInt16 nStepCount ) { mnReducedGradientStepCount = nStepCount; }

    sal_Bool                        IsReduceBitmaps() const { return mbReduceBitmaps; }
    void                        SetReduceBitmaps( sal_Bool bSet ) { mbReduceBitmaps = bSet; }

    PrinterBitmapMode           GetReducedBitmapMode() const { return meReducedBitmapMode; }
    void                        SetReducedBitmapMode( PrinterBitmapMode eMode ) { meReducedBitmapMode = eMode; }

    sal_uInt16                      GetReducedBitmapResolution() const { return mnReducedBitmapResolution; }
    void                        SetReducedBitmapResolution( sal_uInt16 nResolution ) { mnReducedBitmapResolution = nResolution; }

    sal_Bool                        IsReducedBitmapIncludesTransparency() const { return mbReducedBitmapsIncludeTransparency; }
    void                        SetReducedBitmapIncludesTransparency( sal_Bool bSet ) { mbReducedBitmapsIncludeTransparency = bSet; }

    sal_Bool                        IsConvertToGreyscales() const { return mbConvertToGreyscales; }
    void                        SetConvertToGreyscales( sal_Bool bSet ) { mbConvertToGreyscales = bSet; }

    // read printer options from configuration, parameter decides whether the set for
    // print "to printer" or "to file" should be read.
    // returns true if config was read, false if an error occured
    bool                        ReadFromConfig( bool bFile );
};

// -----------
// - Printer -
// -----------

class VCL_DLLPUBLIC Printer : public OutputDevice
{
    friend class OutputDevice;

private:
    SalInfoPrinter*             mpInfoPrinter;
    SalPrinter*                 mpPrinter;
    SalGraphics*                mpJobGraphics;
    Printer*                    mpPrev;
    Printer*                    mpNext;
    VirtualDevice*              mpDisplayDev;
    PrinterOptions*             mpPrinterOptions;
    rtl::OUString                   maPrinterName;
    rtl::OUString                   maDriver;
    rtl::OUString                   maPrintFile;
    rtl::OUString                   maJobName;
    JobSetup                    maJobSetup;
    Point                       maPageOffset;
    Size                        maPaperSize;
    sal_uLong                       mnError;
    sal_uInt16                      mnCurPage;
    sal_uInt16                      mnCurPrintPage;
    sal_uInt16                      mnPageQueueSize;
    sal_uInt16                      mnCopyCount;
    sal_Bool                        mbDefPrinter;
    sal_Bool                        mbPrinting;
    sal_Bool                        mbJobActive;
    sal_Bool                        mbCollateCopy;
    sal_Bool                        mbPrintFile;
    sal_Bool                        mbInPrintPage;
    sal_Bool                        mbNewJobSetup;
    sal_Bool                        mbIsQueuePrinter;
    sal_Bool                        mbUserSetupCompleted;
    sal_Bool                        mbUserSetupResult;
    Link                        maErrorHdl;

    SAL_DLLPRIVATE void         ImplInitData();
    SAL_DLLPRIVATE void         ImplInit( SalPrinterQueueInfo* pInfo );
    SAL_DLLPRIVATE void         ImplInitDisplay( const Window* pWindow );
    SAL_DLLPRIVATE static SalPrinterQueueInfo* ImplGetQueueInfo( const rtl::OUString& rPrinterName,
                                                  const rtl::OUString* pDriver );
    SAL_DLLPRIVATE void         ImplUpdatePageData();
    SAL_DLLPRIVATE void         ImplUpdateFontList();
    SAL_DLLPRIVATE void         ImplFindPaperFormatForUserSize( JobSetup&, bool bMatchNearest );

    SAL_DLLPRIVATE bool StartJob( const rtl::OUString& rJobName, boost::shared_ptr<vcl::PrinterController>& );

    static SAL_DLLPRIVATE sal_uLong ImplSalPrinterErrorCodeToVCL( sal_uLong nError );

private:
    SAL_DLLPRIVATE sal_Bool         EndJob();
    SAL_DLLPRIVATE              Printer( const Printer& rPrinter );
    SAL_DLLPRIVATE Printer&     operator =( const Printer& rPrinter );
public:
    SAL_DLLPRIVATE void         ImplStartPage();
    SAL_DLLPRIVATE void         ImplEndPage();
public:
    void                        DrawGradientEx( OutputDevice* pOut, const Rectangle& rRect, const Gradient& rGradient );

protected:

    void                        SetSelfAsQueuePrinter( sal_Bool bQueuePrinter ) { mbIsQueuePrinter = bQueuePrinter; }
    sal_Bool                        IsQueuePrinter() const { return mbIsQueuePrinter; }

public:
                                Printer();
                                Printer( const JobSetup& rJobSetup );
                                Printer( const QueueInfo& rQueueInfo );
                                Printer( const rtl::OUString& rPrinterName );
    virtual                     ~Printer();

    static const std::vector< rtl::OUString >& GetPrinterQueues();
    static const QueueInfo*     GetQueueInfo( const rtl::OUString& rPrinterName, bool bStatusUpdate );
    static rtl::OUString            GetDefaultPrinterName();

    virtual void                Error();

    const rtl::OUString&            GetName() const             { return maPrinterName; }
    const rtl::OUString&            GetDriverName() const       { return maDriver; }
    sal_Bool                        IsDefPrinter() const        { return mbDefPrinter; }
    sal_Bool                        IsDisplayPrinter() const    { return mpDisplayDev != NULL; }
    sal_Bool                        IsValid() const             { return !IsDisplayPrinter(); }

    sal_uLong                       GetCapabilities( sal_uInt16 nType ) const;
    sal_Bool                        HasSupport( PrinterSupport eFeature ) const;

    sal_Bool                        SetJobSetup( const JobSetup& rSetup );
    const JobSetup&             GetJobSetup() const { return maJobSetup; }
    void                        SetJobValue( const rtl::OUString& rKey, const rtl::OUString& rValue ) { maJobSetup.SetValue( rKey, rValue ); }

    sal_Bool                        Setup( Window* pWindow = NULL );
    sal_Bool                        SetPrinterProps( const Printer* pPrinter );

    // SetPrinterOptions is used internally only now
    // in earlier times it was used only to set the options loaded directly from the configuration
    // in SfxPrinter::InitJob, this is now handled internally
    // should the need arise to set the printer options outside vcl, also a method would have to be devised
    // to not override these again internally
    SAL_DLLPRIVATE void         SetPrinterOptions( const PrinterOptions& rOptions );
    const PrinterOptions&       GetPrinterOptions() const { return( *mpPrinterOptions ); }

    sal_Bool                        SetOrientation( Orientation eOrient );
    Orientation                 GetOrientation() const;
    sal_Bool                        SetDuplexMode( DuplexMode );
    // returns the angle that a landscape page will be turned counterclockwise
    // wrt to portrait. The return value may be only valid for
    // the current paper
    int                         GetLandscapeAngle() const;
    sal_Bool                        SetPaperBin( sal_uInt16 nPaperBin );
    sal_uInt16                      GetPaperBin() const;
    sal_Bool                        SetPaper( Paper ePaper );
    sal_Bool                        SetPaperSizeUser( const Size& rSize );
    sal_Bool                        SetPaperSizeUser( const Size& rSize, bool bMatchNearest );
    Paper                   GetPaper() const;
    static rtl::OUString        GetPaperName( Paper ePaper );
    // return a UI string for the current paper; i_bPaperUser == false means an empty string for PAPER_USER
    rtl::OUString               GetPaperName( bool i_bPaperUser = true ) const;

    // returns number of available paper formats
    int                         GetPaperInfoCount() const;
    // returns info about paper format nPaper
    const PaperInfo&            GetPaperInfo( int nPaper ) const;
    sal_uInt16                      GetPaperBinCount() const;
    rtl::OUString                   GetPaperBinName( sal_uInt16 nPaperBin ) const;

    const Size&                 GetPaperSizePixel() const { return maPaperSize; }
    Size                        GetPaperSize() const { return PixelToLogic( maPaperSize ); }
    const Point&                GetPageOffsetPixel() const { return maPageOffset; }
    Point                       GetPageOffset() const { return PixelToLogic( maPageOffset ); }

    sal_Bool                        SetCopyCount( sal_uInt16 nCopy, sal_Bool bCollate = sal_False );
    sal_uInt16                      GetCopyCount() const { return mnCopyCount; }
    sal_Bool                        IsCollateCopy() const { return mbCollateCopy; }

    sal_Bool                        IsPrinting() const { return mbPrinting; }

    const rtl::OUString&            GetCurJobName() const { return maJobName; }
    sal_uInt16                      GetCurPage() const { return mnCurPage; }
    sal_Bool                        IsJobActive() const { return mbJobActive; }

    sal_uLong                       GetError() const { return ERRCODE_TOERROR(mnError); }
    sal_uLong                       GetErrorCode() const { return mnError; }

    void                        SetErrorHdl( const Link& rLink ) { maErrorHdl = rLink; }
    const Link&                 GetErrorHdl() const { return maErrorHdl; }

    void                        Compat_OldPrinterMetrics( bool bSet );

    /** checks the printer list and updates it necessary
    *
    *   sends a DataChanged event of type DATACHANGED_PRINTER
    *   if the printer list changed
    */
    static void updatePrinters();

    /** execute a print job

        starts a print job asynchronously (that is will return

    */
    static void PrintJob( const boost::shared_ptr<vcl::PrinterController>& i_pController,
                          const JobSetup& i_rInitSetup
                          );

    // implementation detail of PrintJob being asynchronous
    // not exported, not usable outside vcl
    static void SAL_DLLPRIVATE ImplPrintJob( const boost::shared_ptr<vcl::PrinterController>& i_pController,
                                             const JobSetup& i_rInitSetup
                                             );
};

namespace vcl
{
class ImplPrinterControllerData;

class VCL_DLLPUBLIC PrinterController
{
    ImplPrinterControllerData* mpImplData;
protected:
    PrinterController( const boost::shared_ptr<Printer>& );
public:
    enum NupOrderType
    { LRTB, TBLR, TBRL, RLTB };
    struct MultiPageSetup
    {
        // all metrics in 100th mm
        int                                    nRows;
        int                                    nColumns;
        int                                    nRepeat;
        Size                                   aPaperSize;
        long                                   nLeftMargin;
        long                                   nTopMargin;
        long                                   nRightMargin;
        long                                   nBottomMargin;
        long                                   nHorizontalSpacing;
        long                                   nVerticalSpacing;
        bool                                   bDrawBorder;
        PrinterController::NupOrderType        nOrder;

        MultiPageSetup()
        : nRows( 1 ), nColumns( 1 ), nRepeat( 1 ), aPaperSize( 21000, 29700 )
        , nLeftMargin( 0 ), nTopMargin( 0 )
        , nRightMargin( 0 ), nBottomMargin( 0 )
        , nHorizontalSpacing( 0 ), nVerticalSpacing( 0 )
        , bDrawBorder( false )
        , nOrder( LRTB )
        {
        }
    };

    struct PageSize
    {
        Size        aSize;          // in 100th mm
        bool        bFullPaper;     // full paper, not only imageable area is printed

        PageSize( const Size& i_rSize = Size( 21000, 29700 ),
                  bool i_bFullPaper = false
                  ) : aSize( i_rSize ), bFullPaper( i_bFullPaper ) {}
    };

    virtual ~PrinterController();

    const boost::shared_ptr<Printer>& getPrinter() const;
    /* for implementations: get current job properties as changed by e.g. print dialog
       this gets the current set of properties initially told to Printer::PrintJob

       For convenience a second sequence will be merged in to get a combined sequence.
       In case of duplicate property names, the value of i_MergeList wins.
    */
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >
        getJobProperties( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& i_rMergeList ) const;

    /* get the PropertyValue of a Property
    */
    com::sun::star::beans::PropertyValue* getValue( const rtl::OUString& i_rPropertyName );
    const com::sun::star::beans::PropertyValue* getValue( const rtl::OUString& i_rPropertyName ) const;
    /* get a bool property
       in case the property is unknown or not convertible to bool, i_bFallback is returned
    */
    sal_Bool getBoolProperty( const rtl::OUString& i_rPropertyName, sal_Bool i_bFallback ) const;

    /* set a property value - can also be used to add another UI property
    */
    void setValue( const rtl::OUString& i_rPropertyName, const com::sun::star::uno::Any& i_rValue );
    void setValue( const com::sun::star::beans::PropertyValue& i_rValue );

    /* return the currently active UI options. These are the same that were passed to setUIOptions.
    */
    const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& getUIOptions() const;
    /* set possible UI options. should only be done once before passing the PrinterListener
       to Printer::PrintJob
    */
    void setUIOptions( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& );
    /* enable/disable an option; this can be used to implement dialog logic.
    */
    bool isUIOptionEnabled( const rtl::OUString& rPropName ) const;
    bool isUIChoiceEnabled( const rtl::OUString& rPropName, sal_Int32 nChoice ) const;
    /* returns the property name rPropName depends on or an empty string
       if no dependency exists.
    */
    rtl::OUString getDependency( const rtl::OUString& rPropName ) const;
    /* makeEnabled will chage the property rPropName depends on to the value
       that makes rPropName enabled. If the dependency itself is also disabled,
       no action will be performed.

       returns the property name rPropName depends on or an empty string
       if no change was made.
    */
    rtl::OUString makeEnabled( const rtl::OUString& rPropName );

    virtual int  getPageCount() const = 0; // must be overloaded by the app
    /* get the page parameters, namely the jobsetup that should be active for the page
       (describing among others the physical page size) and the "page size". In writer
       case this would probably be the same as the JobSetup since writer sets the page size
       draw/impress for example print their page on the paper set on the printer,
       possibly adjusting the page size to fit. That means the page size can be different from
       the paper size.
    */
    // must be overloaded by the app, return page size in 1/100th mm
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > getPageParameters( int i_nPage ) const = 0;
    virtual void printPage( int i_nPage ) const = 0; // must be overloaded by the app
    virtual void jobStarted(); // will be called after a possible dialog has been shown and the real printjob starts
    virtual void jobFinished( com::sun::star::view::PrintableState );

    com::sun::star::view::PrintableState getJobState() const;

    void abortJob();

    bool isShowDialogs() const;
    bool isDirectPrint() const;

    // implementation details, not usable outside vcl
    // don't use outside vcl. Some of these are exported for
    // the benefit of vcl's plugins.
    // Still: DO NOT USE OUTSIDE VCL
    VCL_PLUGIN_PUBLIC int getFilteredPageCount();
    SAL_DLLPRIVATE PageSize getPageFile( int i_inUnfilteredPage, GDIMetaFile& rMtf, bool i_bMayUseCache = false );
    VCL_PLUGIN_PUBLIC PageSize getFilteredPageFile( int i_nFilteredPage, GDIMetaFile& o_rMtf, bool i_bMayUseCache = false );
    VCL_PLUGIN_PUBLIC void printFilteredPage( int i_nPage );
    SAL_DLLPRIVATE void setPrinter( const boost::shared_ptr<Printer>& );
    SAL_DLLPRIVATE void setOptionChangeHdl( const Link& );
    VCL_PLUGIN_PUBLIC void createProgressDialog();
    VCL_PLUGIN_PUBLIC bool isProgressCanceled() const;
    SAL_DLLPRIVATE void setMultipage( const MultiPageSetup& );
    SAL_DLLPRIVATE const MultiPageSetup& getMultipage() const;
    VCL_PLUGIN_PUBLIC void setLastPage( sal_Bool i_bLastPage );
    SAL_DLLPRIVATE void setReversePrint( sal_Bool i_bReverse );
    SAL_DLLPRIVATE bool getReversePrint() const;
    SAL_DLLPRIVATE void pushPropertiesToPrinter();
    VCL_PLUGIN_PUBLIC void setJobState( com::sun::star::view::PrintableState );
    SAL_DLLPRIVATE bool setupPrinter( Window* i_pDlgParent );

    SAL_DLLPRIVATE int getPageCountProtected() const;
    SAL_DLLPRIVATE com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > getPageParametersProtected( int i_nPage ) const;

    SAL_DLLPRIVATE sal_uLong removeTransparencies( GDIMetaFile& i_rIn, GDIMetaFile& o_rOut );
    SAL_DLLPRIVATE void resetPrinterOptions( bool i_bFileOutput );
};

class VCL_DLLPUBLIC PrinterOptionsHelper
{
    protected:
    boost::unordered_map< rtl::OUString, com::sun::star::uno::Any, rtl::OUStringHash >        m_aPropertyMap;
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >              m_aUIProperties;

    public:
    PrinterOptionsHelper() {} // create without ui properties
    PrinterOptionsHelper( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& i_rUIProperties )
    : m_aUIProperties( i_rUIProperties )
    {}
    ~PrinterOptionsHelper()
    {}

    /* process a new set of properties
     * merges changed properties and returns "true" if any occurred
     * if the optional output set is not NULL then the names of the changed properties are returned
    **/
    bool processProperties( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& i_rNewProp,
                            std::set< rtl::OUString >* o_pChangeProp = NULL );
    /* append  to a sequence of property values the ui property sequence passed at creation
     * as the "ExtraPrintUIOptions" property. if that sequence was empty, no "ExtraPrintUIOptions" property
     * will be appended.
    **/
    void appendPrintUIOptions( com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& io_rProps ) const;

    // returns an empty Any for not existing properties
    com::sun::star::uno::Any getValue( const rtl::OUString& i_rPropertyName ) const;

    sal_Bool getBoolValue( const rtl::OUString& i_rPropertyName, sal_Bool i_bDefault = sal_False ) const;
    // convenience for fixed strings
    sal_Bool getBoolValue( const char* i_pPropName, sal_Bool i_bDefault = sal_False ) const
    { return getBoolValue( rtl::OUString::createFromAscii( i_pPropName ), i_bDefault ); }

    sal_Int64 getIntValue( const rtl::OUString& i_rPropertyName, sal_Int64 i_nDefault = 0 ) const;
    // convenience for fixed strings
    sal_Int64 getIntValue( const char* i_pPropName, sal_Int64 i_nDefault = 0 ) const
    { return getIntValue( rtl::OUString::createFromAscii( i_pPropName ), i_nDefault ); }

    rtl::OUString getStringValue( const rtl::OUString& i_rPropertyName, const rtl::OUString& i_rDefault = rtl::OUString() ) const;
    // convenience for fixed strings
    rtl::OUString getStringValue( const char* i_pPropName, const rtl::OUString& i_rDefault = rtl::OUString() ) const
    { return getStringValue( rtl::OUString::createFromAscii( i_pPropName ), i_rDefault ); }

    // helper functions for user to create a single control
    struct UIControlOptions
    {
        rtl::OUString   maDependsOnName;
        sal_Int32       mnDependsOnEntry;
        sal_Bool        mbAttachToDependency;
        rtl::OUString   maGroupHint;
        sal_Bool        mbInternalOnly;
        sal_Bool        mbEnabled;
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > maAddProps;

        UIControlOptions( const rtl::OUString& i_rDependsOnName = rtl::OUString(),
                          sal_Int32 i_nDependsOnEntry = -1,
                          sal_Bool i_bAttachToDependency = sal_False,
                          const rtl::OUString& i_rGroupHint = rtl::OUString(),
                          sal_Bool i_bInternalOnly = sal_False,
                          sal_Bool i_bEnabled = sal_True
                         )
        : maDependsOnName( i_rDependsOnName )
        , mnDependsOnEntry( i_nDependsOnEntry )
        , mbAttachToDependency( i_bAttachToDependency )
        , maGroupHint( i_rGroupHint )
        , mbInternalOnly( i_bInternalOnly )
        , mbEnabled( i_bEnabled ) {}
    };

    // note: in the following helper functions HelpIds are expected as an rtl::OUString
    // the normal HelpId form is rtl::OString (byte string instead of UTF16 string)
    // this is because the whole interface is base on UNO properties; in fact the structures
    // are passed over UNO interfaces. UNO does not know a byte string, hence the string is
    // transported via UTF16 strings.

    // general control
    static com::sun::star::uno::Any getUIControlOpt( const rtl::OUString& i_rTitle,
                                                     const com::sun::star::uno::Sequence< rtl::OUString >& i_rHelpId,
                                                     const rtl::OUString& i_rType,
                                                     const com::sun::star::beans::PropertyValue* i_pValue = NULL,
                                                     const UIControlOptions& i_rControlOptions = UIControlOptions()
                                                     );
    // create a group (e.g. a TabPage); following controls will be grouped in it until the next
    // group begins
    static com::sun::star::uno::Any getGroupControlOpt( const rtl::OUString& i_rTitle, const rtl::OUString& i_rHelpId );

    // create a subgroup (e.g. a FixedLine); following controls will be grouped in it until the next
    // subgroup or group begins
    // setting bJobPage = true will make the subgroup appear on the first page of the print dialog
    static com::sun::star::uno::Any getSubgroupControlOpt( const rtl::OUString& i_rTitle,
                                                           const rtl::OUString& i_rHelpId,
                                                           const UIControlOptions& i_rControlOptions = UIControlOptions()
                                                           );

    // create a bool option (usually a checkbox)
    static com::sun::star::uno::Any getBoolControlOpt( const rtl::OUString& i_rTitle,
                                                       const rtl::OUString& i_rHelpId,
                                                       const rtl::OUString& i_rProperty,
                                                       sal_Bool i_bValue,
                                                       const UIControlOptions& i_rControlOptions = UIControlOptions()
                                                       );

    // create a set of choices (either a radio button group or a list box)
    static com::sun::star::uno::Any getChoiceControlOpt( const rtl::OUString& i_rTitle,
                                                         const com::sun::star::uno::Sequence< rtl::OUString >& i_rHelpId,
                                                         const rtl::OUString& i_rProperty,
                                                         const com::sun::star::uno::Sequence< rtl::OUString >& i_rChoices,
                                                         sal_Int32 i_nValue,
                                                         const rtl::OUString& i_rType = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Radio" ) ),
                                                         const com::sun::star::uno::Sequence< sal_Bool >& i_rDisabledChoices = com::sun::star::uno::Sequence< sal_Bool >(),
                                                         const UIControlOptions& i_rControlOptions = UIControlOptions()
                                                         );

    // create an integer range (e.g. a spin field)
    // note: max value < min value means do not apply min/max values
    static com::sun::star::uno::Any getRangeControlOpt( const rtl::OUString& i_rTitle,
                                                        const rtl::OUString& i_rHelpId,
                                                        const rtl::OUString& i_rProperty,
                                                        sal_Int32 i_nValue,
                                                        sal_Int32 i_nMinValue = -1,
                                                        sal_Int32 i_nMaxValue = -2,
                                                        const UIControlOptions& i_rControlOptions = UIControlOptions()
                                                        );

    // create a string field
    // note: max value < min value means do not apply min/max values
    static com::sun::star::uno::Any getEditControlOpt( const rtl::OUString& i_rTitle,
                                                       const rtl::OUString& i_rHelpId,
                                                       const rtl::OUString& i_rProperty,
                                                       const rtl::OUString& i_rValue,
                                                       const UIControlOptions& i_rControlOptions = UIControlOptions()
                                                       );
};

}


#endif  // _SV_PRINT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
