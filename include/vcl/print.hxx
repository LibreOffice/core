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

#ifndef INCLUDED_VCL_PRINT_HXX
#define INCLUDED_VCL_PRINT_HXX

#include <rtl/ustring.hxx>

#include <tools/errcode.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>
#include <vcl/prntypes.hxx>
#include <vcl/jobset.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/stream.hxx>
#include <tools/multisel.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/view/PrintableState.hpp>

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


// - Printer-Types -


#define PAGEQUEUE_ALLPAGES   0xFFFF

enum PrinterSupport { SUPPORT_SET_ORIENTATION, SUPPORT_SET_PAPERBIN,
                      SUPPORT_SET_PAPERSIZE, SUPPORT_SET_PAPER,
                      SUPPORT_COPY, SUPPORT_COLLATECOPY,
                      SUPPORT_SETUPDIALOG, SUPPORT_FAX, SUPPORT_PDF };


// - PrinterPage -


class VCL_DLLPUBLIC PrinterPage
{
    GDIMetaFile*    mpMtf;
    JobSetup        maJobSetup;
    bool          mbNewJobSetup;

public:

    PrinterPage() : mpMtf( new GDIMetaFile() ) {}
    PrinterPage( GDIMetaFile* pMtf, bool bNewJobSetup, const JobSetup& rSetup ) :
           mpMtf( pMtf ), maJobSetup( rSetup ), mbNewJobSetup( bNewJobSetup ) {}
    ~PrinterPage() { delete mpMtf; }

    GDIMetaFile*    GetGDIMetaFile() const { return mpMtf; }
    const JobSetup& GetJobSetup() const { return maJobSetup; }
    bool            IsNewJobSetup() const { return mbNewJobSetup; }
};



// - QueueInfo -


class VCL_DLLPUBLIC QueueInfo
{
    friend class Printer;

private:
    OUString                   maPrinterName;
    OUString                   maDriver;
    OUString                   maLocation;
    OUString                   maComment;
    sal_uInt32                  mnStatus;
    sal_uInt32                  mnJobs;

public:
                                QueueInfo();
                                QueueInfo( const QueueInfo& rInfo );
                                ~QueueInfo();

    const OUString&            GetPrinterName() const { return maPrinterName; }
    const OUString&            GetDriver() const { return maDriver; }
    const OUString&            GetLocation() const { return maLocation; }
    const OUString&            GetComment() const { return maComment; }
    sal_uInt32                  GetStatus() const { return mnStatus; }
    sal_uInt32                  GetJobs() const { return mnJobs; }

    bool operator==( const QueueInfo& rInfo ) const;

    friend VCL_DLLPUBLIC SvStream&          ReadQueueInfo( SvStream& rIStream, QueueInfo& rInfo );
    friend VCL_DLLPUBLIC SvStream&          WriteQueueInfo( SvStream& rOStream, const QueueInfo& rInfo );
};


// - PrinterOptions -


enum PrinterTransparencyMode
{
    PRINTER_TRANSPARENCY_AUTO = 0,
    PRINTER_TRANSPARENCY_NONE = 1
};



enum PrinterGradientMode
{
    PRINTER_GRADIENT_STRIPES = 0,
    PRINTER_GRADIENT_COLOR = 1
};



enum PrinterBitmapMode
{
    PRINTER_BITMAP_OPTIMAL = 0,
    PRINTER_BITMAP_NORMAL = 1,
    PRINTER_BITMAP_RESOLUTION = 2
};



class VCL_DLLPUBLIC PrinterOptions
{
private:

    bool                        mbReduceTransparency;
    PrinterTransparencyMode     meReducedTransparencyMode;
    bool                        mbReduceGradients;
    PrinterGradientMode         meReducedGradientsMode;
    sal_uInt16                  mnReducedGradientStepCount;
    bool                        mbReduceBitmaps;
    PrinterBitmapMode           meReducedBitmapMode;
    sal_uInt16                  mnReducedBitmapResolution;
    bool                        mbReducedBitmapsIncludeTransparency;
    bool                        mbConvertToGreyscales;
    bool                        mbPDFAsStandardPrintJobFormat;

public:

                                PrinterOptions();
                                ~PrinterOptions();

    bool                        IsReduceTransparency() const { return mbReduceTransparency; }
    void                        SetReduceTransparency( bool bSet ) { mbReduceTransparency = bSet; }

    PrinterTransparencyMode     GetReducedTransparencyMode() const { return meReducedTransparencyMode; }
    void                        SetReducedTransparencyMode( PrinterTransparencyMode eMode ) { meReducedTransparencyMode = eMode; }

    bool                        IsReduceGradients() const { return mbReduceGradients; }
    void                        SetReduceGradients( bool bSet ) { mbReduceGradients = bSet; }

    PrinterGradientMode         GetReducedGradientMode() const { return meReducedGradientsMode; }
    void                        SetReducedGradientMode( PrinterGradientMode eMode ) { meReducedGradientsMode = eMode; }

    sal_uInt16                  GetReducedGradientStepCount() const { return mnReducedGradientStepCount; }
    void                        SetReducedGradientStepCount( sal_uInt16 nStepCount ) { mnReducedGradientStepCount = nStepCount; }

    bool                        IsReduceBitmaps() const { return mbReduceBitmaps; }
    void                        SetReduceBitmaps( bool bSet ) { mbReduceBitmaps = bSet; }

    PrinterBitmapMode           GetReducedBitmapMode() const { return meReducedBitmapMode; }
    void                        SetReducedBitmapMode( PrinterBitmapMode eMode ) { meReducedBitmapMode = eMode; }

    sal_uInt16                  GetReducedBitmapResolution() const { return mnReducedBitmapResolution; }
    void                        SetReducedBitmapResolution( sal_uInt16 nResolution ) { mnReducedBitmapResolution = nResolution; }

    bool                        IsReducedBitmapIncludesTransparency() const { return mbReducedBitmapsIncludeTransparency; }
    void                        SetReducedBitmapIncludesTransparency( bool bSet ) { mbReducedBitmapsIncludeTransparency = bSet; }

    bool                        IsConvertToGreyscales() const { return mbConvertToGreyscales; }
    void                        SetConvertToGreyscales( bool bSet ) { mbConvertToGreyscales = bSet; }

    bool                        IsPDFAsStandardPrintJobFormat() const { return mbPDFAsStandardPrintJobFormat; }
    void                        SetPDFAsStandardPrintJobFormat( bool bSet ) { mbPDFAsStandardPrintJobFormat = bSet; }

    // read printer options from configuration, parameter decides whether the set for
    // print "to printer" or "to file" should be read.
    // returns true if config was read, false if an error occurred
    bool                        ReadFromConfig( bool bFile );
};


// - Printer -


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
    OUString                    maPrinterName;
    OUString                    maDriver;
    OUString                    maPrintFile;
    OUString                    maJobName;
    JobSetup                    maJobSetup;
    Point                       maPageOffset;
    Size                        maPaperSize;
    sal_uLong                   mnError;
    sal_uInt16                  mnCurPage;
    sal_uInt16                  mnCurPrintPage;
    sal_uInt16                  mnPageQueueSize;
    sal_uInt16                  mnCopyCount;
    bool                        mbDefPrinter;
    bool                        mbPrinting;
    bool                        mbJobActive;
    bool                        mbCollateCopy;
    bool                        mbPrintFile;
    bool                        mbInPrintPage;
    bool                        mbNewJobSetup;
    bool                        mbIsQueuePrinter;
    bool                        mbUserSetupCompleted;
    bool                        mbUserSetupResult;
    Link                        maErrorHdl;

    bool                        ImplInitGraphics() const SAL_OVERRIDE;
    void                        ImplReleaseGraphics( bool bRelease = true ) SAL_OVERRIDE;
    void                        ImplReleaseFonts() SAL_OVERRIDE;
    SAL_DLLPRIVATE void         ImplInitData();
    SAL_DLLPRIVATE void         ImplInit( SalPrinterQueueInfo* pInfo );
    SAL_DLLPRIVATE void         ImplInitDisplay( const Window* pWindow );
    SAL_DLLPRIVATE static SalPrinterQueueInfo* ImplGetQueueInfo( const OUString& rPrinterName,
                                                  const OUString* pDriver );
    SAL_DLLPRIVATE void         ImplUpdatePageData();
    SAL_DLLPRIVATE void         ImplUpdateFontList();
    SAL_DLLPRIVATE void         ImplFindPaperFormatForUserSize( JobSetup&, bool bMatchNearest );

    SAL_DLLPRIVATE bool StartJob( const OUString& rJobName, boost::shared_ptr<vcl::PrinterController>& );

    static SAL_DLLPRIVATE sal_uLong ImplSalPrinterErrorCodeToVCL( sal_uLong nError );

private:
    SAL_DLLPRIVATE bool         EndJob();
    SAL_DLLPRIVATE              Printer( const Printer& rPrinter );
    SAL_DLLPRIVATE Printer&     operator =( const Printer& rPrinter );

public:
    SAL_DLLPRIVATE void         ImplStartPage();
    SAL_DLLPRIVATE void         ImplEndPage();

protected:
    long                        ImplGetGradientStepCount( long nMinRect ) SAL_OVERRIDE;
    void                        ScaleBitmap ( Bitmap&, SalTwoRect& ) SAL_OVERRIDE { };

public:
    void                        DrawGradientEx( OutputDevice* pOut, const Rectangle& rRect, const Gradient& rGradient );

protected:

    void                        SetSelfAsQueuePrinter( bool bQueuePrinter ) { mbIsQueuePrinter = bQueuePrinter; }
    bool                        IsQueuePrinter() const { return mbIsQueuePrinter; }
    virtual void                ImplPrintMask ( const Bitmap& rMask, const Color& rMaskColor,
                                                const Point& rDestPt, const Size& rDestSize,
                                                const Point& rSrcPtPixel, const Size& rSrcSizePixel ) SAL_OVERRIDE;

    bool                        DrawTransformBitmapExDirect(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    const BitmapEx& rBitmapEx) SAL_OVERRIDE;

    bool                        TransformReduceBitmapExTargetRange(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    basegfx::B2DRange &aVisibleRange,
                                    double &fMaximumArea) SAL_OVERRIDE;

    void                        DrawDeviceBitmap(
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                    BitmapEx& rBitmapEx ) SAL_OVERRIDE;

    virtual void                EmulateDrawTransparent( const PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent ) SAL_OVERRIDE;

public:
                                Printer();
                                Printer( const JobSetup& rJobSetup );
                                Printer( const QueueInfo& rQueueInfo );
                                Printer( const OUString& rPrinterName );
    virtual                     ~Printer();

    static const std::vector< OUString >& GetPrinterQueues();
    static const QueueInfo*     GetQueueInfo( const OUString& rPrinterName, bool bStatusUpdate );
    static OUString             GetDefaultPrinterName();

    virtual void                Error();

    const OUString&             GetName() const             { return maPrinterName; }
    const OUString&             GetDriverName() const       { return maDriver; }
    bool                        IsDefPrinter() const        { return mbDefPrinter; }
    bool                        IsDisplayPrinter() const    { return mpDisplayDev != NULL; }
    bool                        IsValid() const             { return !IsDisplayPrinter(); }

    sal_uLong                   GetCapabilities( sal_uInt16 nType ) const;
    bool                        HasSupport( PrinterSupport eFeature ) const;

    bool                        SetJobSetup( const JobSetup& rSetup );
    const JobSetup&             GetJobSetup() const { return maJobSetup; }

    bool                        Setup( Window* pWindow = NULL );
    bool                        SetPrinterProps( const Printer* pPrinter );

    // SetPrinterOptions is used internally only now
    // in earlier times it was used only to set the options loaded directly from the configuration
    // in SfxPrinter::InitJob, this is now handled internally
    // should the need arise to set the printer options outside vcl, also a method would have to be devised
    // to not override these again internally
    SAL_DLLPRIVATE void         SetPrinterOptions( const PrinterOptions& rOptions );
    const PrinterOptions&       GetPrinterOptions() const { return( *mpPrinterOptions ); }

    bool                        SetOrientation( Orientation eOrient );
    Orientation                 GetOrientation() const;
    bool                        SetDuplexMode( DuplexMode );
    // returns the angle that a landscape page will be turned counterclockwise
    // wrt to portrait. The return value may be only valid for
    // the current paper
    int                         GetLandscapeAngle() const;
    bool                        SetPaperBin( sal_uInt16 nPaperBin );
    sal_uInt16                  GetPaperBin() const;
    bool                        SetPaper( Paper ePaper );
    bool                        SetPaperSizeUser( const Size& rSize );
    bool                        SetPaperSizeUser( const Size& rSize, bool bMatchNearest );
    Paper                       GetPaper() const;
    static OUString             GetPaperName( Paper ePaper );
    // return a UI string for the current paper; i_bPaperUser == false means an empty string for PAPER_USER
    OUString                    GetPaperName( bool i_bPaperUser = true ) const;

    // returns number of available paper formats
    int                         GetPaperInfoCount() const;
    // returns info about paper format nPaper
    const PaperInfo&            GetPaperInfo( int nPaper ) const;
    sal_uInt16                  GetPaperBinCount() const;
    OUString                    GetPaperBinName( sal_uInt16 nPaperBin ) const;

    const Size&                 GetPaperSizePixel() const { return maPaperSize; }
    Size                        GetPaperSize() const { return PixelToLogic( maPaperSize ); }
    const Point&                GetPageOffsetPixel() const { return maPageOffset; }
    Point                       GetPageOffset() const { return PixelToLogic( maPageOffset ); }

    bool                        SetCopyCount( sal_uInt16 nCopy, bool bCollate = false );
    sal_uInt16                  GetCopyCount() const { return mnCopyCount; }
    bool                        IsCollateCopy() const { return mbCollateCopy; }

    bool                        IsPrinting() const { return mbPrinting; }

    const OUString&             GetCurJobName() const { return maJobName; }
    sal_uInt16                  GetCurPage() const { return mnCurPage; }
    bool                        IsJobActive() const { return mbJobActive; }

    sal_uLong                   GetError() const { return ERRCODE_TOERROR(mnError); }
    sal_uLong                   GetErrorCode() const { return mnError; }

    void                        SetErrorHdl( const Link& rLink ) { maErrorHdl = rLink; }
    const Link&                 GetErrorHdl() const { return maErrorHdl; }

    /** checks the printer list and updates it necessary
    *
    *   sends a DataChanged event of type DATACHANGED_PRINTER
    *   if the printer list changed
    */
    static void                 updatePrinters();

    /** execute a print job

        starts a print job asynchronously (that is will return

    */
    static void                 PrintJob( const boost::shared_ptr<vcl::PrinterController>& i_pController,
                                          const JobSetup& i_rInitSetup );

    virtual bool                HasMirroredGraphics() const SAL_OVERRIDE;

    virtual void                DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                                            const Point& rSrcPt,  const Size& rSrcSize ) SAL_OVERRIDE;
    virtual void                DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                                            const Point& rSrcPt,  const Size& rSrcSize,
                                            const OutputDevice& rOutDev ) SAL_OVERRIDE;
    virtual void                CopyArea( const Point& rDestPt,
                                          const Point& rSrcPt,  const Size& rSrcSize,
                                          sal_uInt16 nFlags = 0 ) SAL_OVERRIDE;


    // implementation detail of PrintJob being asynchronous
    // not exported, not usable outside vcl
    static void SAL_DLLPRIVATE  ImplPrintJob( const boost::shared_ptr<vcl::PrinterController>& i_pController,
                                              const JobSetup& i_rInitSetup );
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
    css::uno::Sequence< css::beans::PropertyValue >
        getJobProperties( const css::uno::Sequence< css::beans::PropertyValue >& i_rMergeList ) const;

    /* get the PropertyValue of a Property
    */
    css::beans::PropertyValue* getValue( const OUString& i_rPropertyName );
    const css::beans::PropertyValue* getValue( const OUString& i_rPropertyName ) const;
    /* get a bool property
       in case the property is unknown or not convertible to bool, i_bFallback is returned
    */
    bool getBoolProperty( const OUString& i_rPropertyName, bool i_bFallback ) const;
    /* get an int property
       in case the property is unknown or not convertible to bool, i_nFallback is returned
    */
    sal_Int32 getIntProperty( const OUString& i_rPropertyName, sal_Int32 i_nFallback ) const;

    /* set a property value - can also be used to add another UI property
    */
    void setValue( const OUString& i_rPropertyName, const css::uno::Any& i_rValue );
    void setValue( const css::beans::PropertyValue& i_rValue );

    /* return the currently active UI options. These are the same that were passed to setUIOptions.
    */
    const css::uno::Sequence< css::beans::PropertyValue >& getUIOptions() const;
    /* set possible UI options. should only be done once before passing the PrinterListener
       to Printer::PrintJob
    */
    void setUIOptions( const css::uno::Sequence< css::beans::PropertyValue >& );
    /* enable/disable an option; this can be used to implement dialog logic.
    */
    bool isUIOptionEnabled( const OUString& rPropName ) const;
    bool isUIChoiceEnabled( const OUString& rPropName, sal_Int32 nChoice ) const;
    /* returns the property name rPropName depends on or an empty string
       if no dependency exists.
    */
    OUString getDependency( const OUString& rPropName ) const;
    /* makeEnabled will chage the property rPropName depends on to the value
       that makes rPropName enabled. If the dependency itself is also disabled,
       no action will be performed.

       returns the property name rPropName depends on or an empty string
       if no change was made.
    */
    OUString makeEnabled( const OUString& rPropName );

    virtual int  getPageCount() const = 0; // must be overloaded by the app
    /* get the page parameters, namely the jobsetup that should be active for the page
       (describing among others the physical page size) and the "page size". In writer
       case this would probably be the same as the JobSetup since writer sets the page size
       draw/impress for example print their page on the paper set on the printer,
       possibly adjusting the page size to fit. That means the page size can be different from
       the paper size.
    */
    // must be overloaded by the app, return page size in 1/100th mm
    virtual css::uno::Sequence< css::beans::PropertyValue > getPageParameters( int i_nPage ) const = 0;
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
    VCL_PLUGIN_PUBLIC void setLastPage( bool i_bLastPage );
    SAL_DLLPRIVATE void setReversePrint( bool i_bReverse );
    SAL_DLLPRIVATE bool getReversePrint() const;
    SAL_DLLPRIVATE void setPapersizeFromSetup( bool i_bPapersizeFromSetup );
    SAL_DLLPRIVATE bool getPapersizeFromSetup() const;
    SAL_DLLPRIVATE void pushPropertiesToPrinter();
    SAL_DLLPRIVATE void resetPaperToLastConfigured();
    VCL_PLUGIN_PUBLIC void setJobState( com::sun::star::view::PrintableState );
    SAL_DLLPRIVATE bool setupPrinter( Window* i_pDlgParent );

    SAL_DLLPRIVATE int getPageCountProtected() const;
    SAL_DLLPRIVATE css::uno::Sequence< css::beans::PropertyValue > getPageParametersProtected( int i_nPage ) const;

    SAL_DLLPRIVATE sal_uLong removeTransparencies( GDIMetaFile& i_rIn, GDIMetaFile& o_rOut );
    SAL_DLLPRIVATE void resetPrinterOptions( bool i_bFileOutput );
};

class VCL_DLLPUBLIC PrinterOptionsHelper
{
    protected:
    boost::unordered_map< OUString, css::uno::Any, OUStringHash >        m_aPropertyMap;
    css::uno::Sequence< css::beans::PropertyValue >              m_aUIProperties;

    public:
    PrinterOptionsHelper() {} // create without ui properties
    PrinterOptionsHelper( const css::uno::Sequence< css::beans::PropertyValue >& i_rUIProperties )
    : m_aUIProperties( i_rUIProperties )
    {}
    ~PrinterOptionsHelper()
    {}

    /* process a new set of properties
     * merges changed properties and returns "true" if any occurred
     * if the optional output set is not NULL then the names of the changed properties are returned
    **/
    bool processProperties( const css::uno::Sequence< css::beans::PropertyValue >& i_rNewProp,
                            std::set< OUString >* o_pChangeProp = NULL );
    /* append  to a sequence of property values the ui property sequence passed at creation
     * as the "ExtraPrintUIOptions" property. if that sequence was empty, no "ExtraPrintUIOptions" property
     * will be appended.
    **/
    void appendPrintUIOptions( css::uno::Sequence< css::beans::PropertyValue >& io_rProps ) const;

    // returns an empty Any for not existing properties
    css::uno::Any getValue( const OUString& i_rPropertyName ) const;

    bool getBoolValue( const OUString& i_rPropertyName, bool i_bDefault = false ) const;
    // convenience for fixed strings
    bool getBoolValue( const char* i_pPropName, bool i_bDefault = false ) const
    { return getBoolValue( OUString::createFromAscii( i_pPropName ), i_bDefault ); }

    sal_Int64 getIntValue( const OUString& i_rPropertyName, sal_Int64 i_nDefault = 0 ) const;
    // convenience for fixed strings
    sal_Int64 getIntValue( const char* i_pPropName, sal_Int64 i_nDefault = 0 ) const
    { return getIntValue( OUString::createFromAscii( i_pPropName ), i_nDefault ); }

    OUString getStringValue( const OUString& i_rPropertyName, const OUString& i_rDefault = OUString() ) const;
    // convenience for fixed strings
    OUString getStringValue( const char* i_pPropName, const OUString& i_rDefault = OUString() ) const
    { return getStringValue( OUString::createFromAscii( i_pPropName ), i_rDefault ); }

    // helper functions for user to create a single control
    struct UIControlOptions
    {
        OUString   maDependsOnName;
        sal_Int32       mnDependsOnEntry;
        bool        mbAttachToDependency;
        OUString   maGroupHint;
        bool        mbInternalOnly;
        bool        mbEnabled;
        css::uno::Sequence< css::beans::PropertyValue > maAddProps;

        UIControlOptions( const OUString& i_rDependsOnName = OUString(),
                          sal_Int32 i_nDependsOnEntry = -1,
                          bool i_bAttachToDependency = false,
                          const OUString& i_rGroupHint = OUString(),
                          bool i_bInternalOnly = false,
                          bool i_bEnabled = true
                         )
        : maDependsOnName( i_rDependsOnName )
        , mnDependsOnEntry( i_nDependsOnEntry )
        , mbAttachToDependency( i_bAttachToDependency )
        , maGroupHint( i_rGroupHint )
        , mbInternalOnly( i_bInternalOnly )
        , mbEnabled( i_bEnabled ) {}
    };

    // note: in the following helper functions HelpIds are expected as an OUString
    // the normal HelpId form is OString (byte string instead of UTF16 string)
    // this is because the whole interface is base on UNO properties; in fact the structures
    // are passed over UNO interfaces. UNO does not know a byte string, hence the string is
    // transported via UTF16 strings.

    // Show general control
    static css::uno::Any setUIControlOpt( const css::uno::Sequence< OUString >& i_rIDs,
                                                     const OUString& i_rTitle,
                                                     const css::uno::Sequence< OUString >& i_rHelpId,
                                                     const OUString& i_rType,
                                                     const css::beans::PropertyValue* i_pValue = NULL,
                                                     const UIControlOptions& i_rControlOptions = UIControlOptions()
                                                     );

    // Show and set the title of a TagPage of id i_rID
    static css::uno::Any setGroupControlOpt(const OUString& i_rID,
                                                       const OUString& i_rTitle,
                                                       const OUString& i_rHelpId);

    // Show and set the label of a VclFrame of id i_rID
    static css::uno::Any setSubgroupControlOpt(const OUString& i_rID,
                                                          const OUString& i_rTitle,
                                                          const OUString& i_rHelpId,
                                                          const UIControlOptions& i_rControlOptions = UIControlOptions());

    // Show a bool option as a checkbox
    static css::uno::Any setBoolControlOpt(const OUString& i_rID,
                                                      const OUString& i_rTitle,
                                                      const OUString& i_rHelpId,
                                                      const OUString& i_rProperty,
                                                      bool i_bValue,
                                                      const UIControlOptions& i_rControlOptions = UIControlOptions());

    // Show a set of choices in a list box
    static css::uno::Any setChoiceListControlOpt(const OUString& i_rID,
                                                            const OUString& i_rTitle,
                                                            const css::uno::Sequence< OUString >& i_rHelpId,
                                                            const OUString& i_rProperty,
                                                            const css::uno::Sequence< OUString >& i_rChoices,
                                                            sal_Int32 i_nValue,
                                                            const css::uno::Sequence< sal_Bool >& i_rDisabledChoices = css::uno::Sequence< sal_Bool >(),
                                                            const UIControlOptions& i_rControlOptions = UIControlOptions());

    // show a set of choices as radio buttons
    static css::uno::Any setChoiceRadiosControlOpt(const css::uno::Sequence< OUString >& i_rIDs,
                                                            const OUString& i_rTitle,
                                                            const css::uno::Sequence< OUString >& i_rHelpId,
                                                            const OUString& i_rProperty,
                                                            const css::uno::Sequence< OUString >& i_rChoices,
                                                            sal_Int32 i_nValue,
                                                            const css::uno::Sequence< sal_Bool >& i_rDisabledChoices = css::uno::Sequence< sal_Bool >(),
                                                            const UIControlOptions& i_rControlOptions = UIControlOptions());


    // show an integer range (e.g. a spin field)
    // note: max value < min value means do not apply min/max values
    static css::uno::Any setRangeControlOpt(const OUString& i_rID,
                                                       const OUString& i_rTitle,
                                                       const OUString& i_rHelpId,
                                                       const OUString& i_rProperty,
                                                       sal_Int32 i_nValue,
                                                       sal_Int32 i_nMinValue = -1,
                                                       sal_Int32 i_nMaxValue = -2,
                                                       const UIControlOptions& i_rControlOptions = UIControlOptions());

    // show a string field
    // note: max value < min value means do not apply min/max values
    static css::uno::Any setEditControlOpt(const OUString& i_rID,
                                                      const OUString& i_rTitle,
                                                      const OUString& i_rHelpId,
                                                      const OUString& i_rProperty,
                                                      const OUString& i_rValue,
                                                      const UIControlOptions& i_rControlOptions = UIControlOptions());
};

}


#endif // INCLUDED_VCL_PRINT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
