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

#include <memory>
#include <set>
#include <unordered_map>

class SalInfoPrinter;
struct SalPrinterQueueInfo;
class SalPrinter;
class VirtualDevice;
namespace vcl { class Window; }

namespace vcl {
    class PrinterController;
    class PrintDialog;
}



enum PrinterSupport { SUPPORT_SET_ORIENTATION, SUPPORT_SET_PAPERBIN,
                      SUPPORT_SET_PAPERSIZE, SUPPORT_SET_PAPER,
                      SUPPORT_COPY, SUPPORT_COLLATECOPY,
                      SUPPORT_SETUPDIALOG, SUPPORT_FAX, SUPPORT_PDF };


class VCL_DLLPUBLIC PrinterPage
{
    GDIMetaFile*    mpMtf;
    JobSetup        maJobSetup;

public:

    PrinterPage() : mpMtf( new GDIMetaFile() ) {}
    PrinterPage( GDIMetaFile* pMtf, const JobSetup& rSetup ) :
           mpMtf( pMtf ), maJobSetup( rSetup ) {}
    ~PrinterPage() { delete mpMtf; }

};



class VCL_DLLPUBLIC QueueInfo
{
    friend class Printer;

private:
    OUString                   maPrinterName;
    OUString                   maDriver;
    OUString                   maLocation;
    OUString                   maComment;
    PrintQueueFlags            mnStatus;
    sal_uInt32                 mnJobs;

public:
                               QueueInfo();
                               QueueInfo( const QueueInfo& rInfo );
                               ~QueueInfo();

    const OUString&            GetPrinterName() const { return maPrinterName; }
    const OUString&            GetDriver() const { return maDriver; }
    const OUString&            GetLocation() const { return maLocation; }
    const OUString&            GetComment() const { return maComment; }
    PrintQueueFlags            GetStatus() const { return mnStatus; }
    sal_uInt32                 GetJobs() const { return mnJobs; }

    bool operator==( const QueueInfo& rInfo ) const;
};


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


class VCL_DLLPUBLIC Printer : public OutputDevice
{
    friend class ::OutputDevice;

private:
    SalInfoPrinter*             mpInfoPrinter;
    SalPrinter*                 mpPrinter;
    SalGraphics*                mpJobGraphics;
    VclPtr<Printer>             mpPrev;
    VclPtr<Printer>             mpNext;
    VclPtr<VirtualDevice>       mpDisplayDev;
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

    SAL_DLLPRIVATE void         ImplInitData();
    SAL_DLLPRIVATE void         ImplInit( SalPrinterQueueInfo* pInfo );
    SAL_DLLPRIVATE void         ImplInitDisplay( const vcl::Window* pWindow );
    SAL_DLLPRIVATE static SalPrinterQueueInfo* ImplGetQueueInfo( const OUString& rPrinterName,
                                                  const OUString* pDriver );
    SAL_DLLPRIVATE void         ImplUpdatePageData();
    SAL_DLLPRIVATE void         ImplUpdateFontList();
    SAL_DLLPRIVATE void         ImplFindPaperFormatForUserSize( JobSetup&, bool bMatchNearest );

    SAL_DLLPRIVATE bool StartJob( const OUString& rJobName, std::shared_ptr<vcl::PrinterController>& );

    static SAL_DLLPRIVATE sal_uLong ImplSalPrinterErrorCodeToVCL( sal_uLong nError );

private:
    SAL_DLLPRIVATE bool         EndJob();
                                Printer( const Printer& rPrinter ) = delete;
                   Printer&     operator =( const Printer& rPrinter ) = delete;

public:
    SAL_DLLPRIVATE void         ImplStartPage();
    SAL_DLLPRIVATE void         ImplEndPage();

protected:
    virtual bool                AcquireGraphics() const override;
    virtual void                ReleaseGraphics( bool bRelease = true ) override;
    virtual void                ImplReleaseFonts() override;

    virtual long                GetGradientStepCount( long nMinRect ) override;
    virtual bool                UsePolyPolygonForComplexGradient() override;
    virtual void                ClipAndDrawGradientMetafile ( const Gradient &rGradient, const tools::PolyPolygon &rPolyPoly ) override;

    void                        ScaleBitmap ( Bitmap&, SalTwoRect& ) override { };

public:
    void                        DrawGradientEx( OutputDevice* pOut, const Rectangle& rRect, const Gradient& rGradient );
    virtual Bitmap              GetBitmap( const Point& rSrcPt, const Size& rSize ) const override;

protected:
    virtual void                DrawDeviceMask ( const Bitmap& rMask, const Color& rMaskColor,
                                            const Point& rDestPt, const Size& rDestSize,
                                            const Point& rSrcPtPixel, const Size& rSrcSizePixel ) override;

    bool                        DrawTransformBitmapExDirect(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    const BitmapEx& rBitmapEx) override;

    bool                        TransformAndReduceBitmapExToTargetRange(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    basegfx::B2DRange &aVisibleRange,
                                    double &fMaximumArea) override;

    void                        DrawDeviceBitmap(
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                    BitmapEx& rBitmapEx ) override;

    virtual void                EmulateDrawTransparent( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent ) override;

    virtual void                InitFont() const override;
    virtual void                SetFontOrientation( ImplFontEntry* const pFontEntry ) const override;

public:
                                Printer();
                                Printer( const JobSetup& rJobSetup );
                                Printer( const QueueInfo& rQueueInfo );
                                Printer( const OUString& rPrinterName );
    virtual                     ~Printer();
    virtual void                dispose() override;

    static const std::vector< OUString >& GetPrinterQueues();
    static const QueueInfo*     GetQueueInfo( const OUString& rPrinterName, bool bStatusUpdate );
    static OUString             GetDefaultPrinterName();

    const OUString&             GetName() const             { return maPrinterName; }
    const OUString&             GetDriverName() const       { return maDriver; }
    bool                        IsDefPrinter() const        { return mbDefPrinter; }
    bool                        IsDisplayPrinter() const    { return mpDisplayDev != nullptr; }
    bool                        IsValid() const             { return !IsDisplayPrinter(); }

    sal_uLong                   GetCapabilities( PrinterCapType nType ) const;
    bool                        HasSupport( PrinterSupport eFeature ) const;

    bool                        SetJobSetup( const JobSetup& rSetup );
    const JobSetup&             GetJobSetup() const { return maJobSetup; }

    bool                        Setup( vcl::Window* pWindow = NULL, bool bPapersizeFromSetup = false );
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

    bool                        IsJobActive() const { return mbJobActive; }

    /** checks the printer list and updates it necessary
    *
    *   sends a DataChanged event of type DataChangedEventType::PRINTER
    *   if the printer list changed
    */
    static void                 updatePrinters();

    /** execute a print job

        starts a print job asynchronously (that is will return

    */
    static void                 PrintJob( const std::shared_ptr<vcl::PrinterController>& i_pController,
                                          const JobSetup& i_rInitSetup );

    virtual bool                HasMirroredGraphics() const override;

    virtual void                DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                                            const Point& rSrcPt,  const Size& rSrcSize ) override;
    virtual void                DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                                            const Point& rSrcPt,  const Size& rSrcSize,
                                            const OutputDevice& rOutDev ) override;
    virtual void                CopyArea( const Point& rDestPt,
                                          const Point& rSrcPt,  const Size& rSrcSize,
                                          bool bWindowInvalidate = false ) override;

    virtual void                DrawImage( const Point&, const Image&, DrawImageFlags ) override;
    virtual void                DrawImage( const Point&, const Size&,
                                           const Image&, DrawImageFlags ) override;


    // These 3 together are more modular PrintJob(), allowing printing more documents as one print job
    // by repeated calls to ExecutePrintJob(). Used by mailmerge.
    static bool                 PreparePrintJob( std::shared_ptr<vcl::PrinterController> i_pController,
                                        const JobSetup& i_rInitSetup );
    static bool                 ExecutePrintJob( std::shared_ptr<vcl::PrinterController> i_pController );
    static void                 FinishPrintJob( std::shared_ptr<vcl::PrinterController> i_pController );

    // implementation detail of PrintJob being asynchronous
    // not exported, not usable outside vcl
    static void SAL_DLLPRIVATE  ImplPrintJob( std::shared_ptr<vcl::PrinterController> i_pController,
                                              const JobSetup& i_rInitSetup );
};

namespace vcl
{
class ImplPrinterControllerData;

class VCL_DLLPUBLIC PrinterController
{
    ImplPrinterControllerData* mpImplData;
protected:
    PrinterController( const VclPtr<Printer>& );
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

    const VclPtr<Printer>& getPrinter() const;
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

    virtual int getPageCount() const = 0; /// App must override this
    /* get the page parameters, namely the jobsetup that should be active for the page
       (describing among others the physical page size) and the "page size". In writer
       case this would probably be the same as the JobSetup since writer sets the page size
       draw/impress for example print their page on the paper set on the printer,
       possibly adjusting the page size to fit. That means the page size can be different from
       the paper size.
       App must override this, return page size in 1/100th mm
    */
    virtual css::uno::Sequence< css::beans::PropertyValue > getPageParameters( int i_nPage ) const = 0;
    virtual void printPage(int i_nPage) const = 0; /// App must override this
    virtual void jobStarted(); // will be called after a possible dialog has been shown and the real printjob starts
    virtual void jobFinished( css::view::PrintableState );

    css::view::PrintableState getJobState() const;

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
    SAL_DLLPRIVATE void setPrinter( const VclPtr<Printer>& );
    VCL_PLUGIN_PUBLIC void createProgressDialog();
    VCL_PLUGIN_PUBLIC bool isProgressCanceled() const;
    SAL_DLLPRIVATE void setMultipage( const MultiPageSetup& );
    SAL_DLLPRIVATE const MultiPageSetup& getMultipage() const;
    VCL_PLUGIN_PUBLIC void setLastPage( bool i_bLastPage );
    SAL_DLLPRIVATE void setReversePrint( bool i_bReverse );
    SAL_DLLPRIVATE bool getReversePrint() const;
    SAL_DLLPRIVATE void setPapersizeFromSetup( bool i_bPapersizeFromSetup );
    SAL_DLLPRIVATE bool getPapersizeFromSetup() const;
    VCL_PLUGIN_PUBLIC void setPrinterModified( bool i_bPapersizeFromSetup );
    VCL_PLUGIN_PUBLIC bool getPrinterModified() const;
    SAL_DLLPRIVATE void pushPropertiesToPrinter();
    SAL_DLLPRIVATE void resetPaperToLastConfigured();
    VCL_PLUGIN_PUBLIC void setJobState( css::view::PrintableState );
    SAL_DLLPRIVATE bool setupPrinter( vcl::Window* i_pDlgParent );

    SAL_DLLPRIVATE int getPageCountProtected() const;
    SAL_DLLPRIVATE css::uno::Sequence< css::beans::PropertyValue > getPageParametersProtected( int i_nPage ) const;

    SAL_DLLPRIVATE DrawModeFlags removeTransparencies( GDIMetaFile& i_rIn, GDIMetaFile& o_rOut );
    SAL_DLLPRIVATE void resetPrinterOptions( bool i_bFileOutput );
};

class VCL_DLLPUBLIC PrinterOptionsHelper
{
    protected:
    std::unordered_map< OUString, css::uno::Any, OUStringHash >  m_aPropertyMap;
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

    sal_Int64 getIntValue( const OUString& i_rPropertyName, sal_Int64 i_nDefault ) const;
    // convenience for fixed strings
    sal_Int64 getIntValue( const char* i_pPropName, sal_Int64 i_nDefault ) const
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
