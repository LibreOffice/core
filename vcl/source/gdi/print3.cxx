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

#include <vcl/layout.hxx>
#include <vcl/print.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/unohelp.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <sal/types.h>

#include "printdlg.hxx"
#include "svdata.hxx"
#include "salinst.hxx"
#include "salprn.hxx"
#include "svids.hrc"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/view/DuplexMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <unordered_map>
#include <unordered_set>

using namespace vcl;

class ImplPageCache
{
    struct CacheEntry
    {
        GDIMetaFile                 aPage;
        PrinterController::PageSize aSize;
    };

    std::vector< CacheEntry >   maPages;
    std::vector< sal_Int32 >    maPageNumbers;
    std::vector< sal_Int32 >    maCacheRanking;

    static const sal_Int32      nCacheSize = 6;

    void updateRanking( sal_Int32 nLastHit )
    {
        if( maCacheRanking[0] != nLastHit )
        {
            for( sal_Int32 i = nCacheSize-1; i > 0; i-- )
                maCacheRanking[i] = maCacheRanking[i-1];
            maCacheRanking[0] = nLastHit;
        }
    }

public:
    ImplPageCache()
    : maPages( nCacheSize )
    , maPageNumbers( nCacheSize, -1 )
    , maCacheRanking( nCacheSize )
    {
        for( sal_Int32 i = 0; i < nCacheSize; i++ )
            maCacheRanking[i] = nCacheSize - i - 1;
    }

    // caution: does not ensure uniqueness
    void insert( sal_Int32 i_nPageNo, const GDIMetaFile& i_rPage, const PrinterController::PageSize& i_rSize )
    {
        sal_Int32 nReplacePage = maCacheRanking.back();
        maPages[ nReplacePage ].aPage = i_rPage;
        maPages[ nReplacePage ].aSize = i_rSize;
        maPageNumbers[ nReplacePage ] = i_nPageNo;
        // cache insertion means in our case, the page was just queried
        // so update the ranking
        updateRanking( nReplacePage );
    }

    // caution: bad algorithm; should there ever be reason to increase the cache size beyond 6
    // this needs to be urgently rewritten. However do NOT increase the cache size lightly,
    // whole pages can be rather memory intensive
    bool get( sal_Int32 i_nPageNo, GDIMetaFile& o_rPageFile, PrinterController::PageSize& o_rSize )
    {
        for( sal_Int32 i = 0; i < nCacheSize; ++i )
        {
            if( maPageNumbers[i] == i_nPageNo )
            {
                updateRanking( i );
                o_rPageFile = maPages[i].aPage;
                o_rSize = maPages[i].aSize;
                return true;
            }
        }
        return false;
    }

    void invalidate()
    {
        for( sal_Int32 i = 0; i < nCacheSize; ++i )
        {
            maPageNumbers[i] = -1;
            maPages[i].aPage.Clear();
            maCacheRanking[i] = nCacheSize - i - 1;
        }
    }
};

class vcl::ImplPrinterControllerData
{
public:
    struct ControlDependency
    {
        OUString       maDependsOnName;
        sal_Int32           mnDependsOnEntry;

        ControlDependency() : mnDependsOnEntry( -1 ) {}
    };

    typedef std::unordered_map< OUString, size_t, OUStringHash > PropertyToIndexMap;
    typedef std::unordered_map< OUString, ControlDependency, OUStringHash > ControlDependencyMap;
    typedef std::unordered_map< OUString, css::uno::Sequence< sal_Bool >, OUStringHash > ChoiceDisableMap;

    VclPtr< Printer >                                           mxPrinter;
    css::uno::Sequence< css::beans::PropertyValue >             maUIOptions;
    std::vector< css::beans::PropertyValue >                    maUIProperties;
    std::vector< bool >                                         maUIPropertyEnabled;
    PropertyToIndexMap                                          maPropertyToIndex;
    ControlDependencyMap                                        maControlDependencies;
    ChoiceDisableMap                                            maChoiceDisableMap;
    bool                                                        mbFirstPage;
    bool                                                        mbLastPage;
    bool                                                        mbReversePageOrder;
    bool                                                        mbPapersizeFromSetup;
    bool                                                        mbPrinterModified;
    css::view::PrintableState                                   meJobState;

    vcl::PrinterController::MultiPageSetup                      maMultiPage;

    VclPtr<vcl::PrintProgressDialog>                            mpProgress;

    ImplPageCache                                               maPageCache;

    // set by user through printer properties subdialog of printer settings dialog
    Size                                                        maDefaultPageSize;
    // set by user through printer properties subdialog of printer settings dialog
    sal_Int32                                                   mnDefaultPaperBin;
    // Set by user through printer properties subdialog of print dialog.
    // Overrides application-set tray for a page.
    sal_Int32                                                   mnFixedPaperBin;

    // N.B. Apparently we have three levels of paper tray settings
    // (latter overrides former):
    // 1. default tray
    // 2. tray set for a concrete page by an application, e.g., writer
    //    allows setting a printer tray (for the default printer) for a
    //    page style. This setting can be overridden by user by selecting
    //    "Use only paper tray from printer preferences" on the Options
    //    page in the print dialog, in which case the default tray is
    //    used for all pages.
    // 3. tray set in printer properties the printer dialog
    // I'm not quite sure why 1. and 3. are distinct, but the commit
    // history suggests this is intentional...

    ImplPrinterControllerData() :
        mbFirstPage( true ),
        mbLastPage( false ),
        mbReversePageOrder( false ),
        mbPapersizeFromSetup( false ),
        mbPrinterModified( false ),
        meJobState( css::view::PrintableState_JOB_STARTED ),
        mpProgress( nullptr ),
        mnDefaultPaperBin( -1 ),
        mnFixedPaperBin( -1 )
    {}
    ~ImplPrinterControllerData() { mpProgress.disposeAndClear(); }

    const Size& getRealPaperSize( const Size& i_rPageSize, bool bNoNUP ) const
    {
        if( mbPapersizeFromSetup )
            return maDefaultPageSize;
        if( maMultiPage.nRows * maMultiPage.nColumns > 1 && ! bNoNUP )
            return maMultiPage.aPaperSize;
        return i_rPageSize;
    }
    bool isFixedPageSize() const
    { return mbPapersizeFromSetup; }
    PrinterController::PageSize modifyJobSetup( const css::uno::Sequence< css::beans::PropertyValue >& i_rProps );
    void resetPaperToLastConfigured();
};

PrinterController::PrinterController( const VclPtr<Printer>& i_xPrinter )
    : mpImplData( new ImplPrinterControllerData )
{
    mpImplData->mxPrinter = i_xPrinter;
}

static OUString queryFile( Printer* pPrinter )
{
    OUString aResult;

    css::uno::Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    css::uno::Reference< css::ui::dialogs::XFilePicker3 > xFilePicker = css::ui::dialogs::FilePicker::createWithMode(xContext, css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION);

    try
    {
#ifdef UNX
        // add PostScript and PDF
        bool bPS = true, bPDF = true;
        if( pPrinter )
        {
            if( pPrinter->GetCapabilities( PrinterCapType::PDF ) )
                bPS = false;
            else
                bPDF = false;
        }
        if( bPS )
            xFilePicker->appendFilter( "PostScript", "*.ps" );
        if( bPDF )
            xFilePicker->appendFilter( "Portable Document Format", "*.pdf" );
#elif defined WNT
        (void)pPrinter;
        xFilePicker->appendFilter( "*.PRN", "*.prn" );
#endif
        // add arbitrary files
        xFilePicker->appendFilter(VclResId(SV_STDTEXT_ALLFILETYPES), "*.*");
    }
    catch (const css::lang::IllegalArgumentException&)
    {
        SAL_WARN( "vcl.gdi", "caught IllegalArgumentException when registering filter" );
    }

    if( xFilePicker->execute() == css::ui::dialogs::ExecutableDialogResults::OK )
    {
        css::uno::Sequence< OUString > aPathSeq( xFilePicker->getSelectedFiles() );
        INetURLObject aObj( aPathSeq[0] );
        aResult = aObj.PathToFileName();
    }
    return aResult;
}

struct PrintJobAsync
{
    std::shared_ptr<PrinterController>  mxController;
    JobSetup                            maInitSetup;

    PrintJobAsync(const std::shared_ptr<PrinterController>& i_xController,
                  const JobSetup& i_rInitSetup)
    : mxController( i_xController ), maInitSetup( i_rInitSetup )
    {}

    DECL_LINK( ExecJob, void*, void );
};

IMPL_LINK_NOARG(PrintJobAsync, ExecJob, void*, void)
{
    Printer::ImplPrintJob(mxController, maInitSetup);

    // clean up, do not access members after this
    delete this;
}

void Printer::PrintJob(const std::shared_ptr<PrinterController>& i_xController,
                       const JobSetup& i_rInitSetup)
{
    bool bSynchronous = false;
    css::beans::PropertyValue* pVal = i_xController->getValue( OUString( "Wait" ) );
    if( pVal )
        pVal->Value >>= bSynchronous;

    if( bSynchronous )
        ImplPrintJob(i_xController, i_rInitSetup);
    else
    {
        PrintJobAsync* pAsync = new PrintJobAsync(i_xController, i_rInitSetup);
        Application::PostUserEvent( LINK( pAsync, PrintJobAsync, ExecJob ) );
    }
}

bool Printer::PreparePrintJob(std::shared_ptr<PrinterController> xController,
                           const JobSetup& i_rInitSetup)
{
    // check if there is a default printer; if not, show an error box (if appropriate)
    if( GetDefaultPrinterName().isEmpty() )
    {
        if (xController->isShowDialogs())
        {
            ScopedVclPtrInstance<MessageDialog> aBox(
                nullptr, "ErrorNoPrinterDialog",
                "vcl/ui/errornoprinterdialog.ui");
            aBox->Execute();
        }
        xController->setValue( "IsDirect",
                               css::uno::makeAny( false ) );
    }

    // setup printer

    // #i114306# changed behavior back from persistence
    // if no specific printer is already set, create the default printer
    if (!xController->getPrinter())
    {
        OUString aPrinterName( i_rInitSetup.GetPrinterName() );
        VclPtrInstance<Printer> xPrinter( aPrinterName );
        xPrinter->SetJobSetup(i_rInitSetup);
        xController->setPrinter(xPrinter);
    }

    // reset last page property
    xController->setLastPage(false);

    // update "PageRange" property inferring from other properties:
    // case 1: "Pages" set from UNO API ->
    //         setup "Print Selection" and insert "PageRange" attribute
    // case 2: "All pages" is selected
    //         update "Page range" attribute to have a sensible default,
    //         but leave "All" as selected

    // "Pages" attribute from API is now equivalent to "PageRange"
    // AND "PrintContent" = 1 except calc where it is "PrintRange" = 1
    // Argh ! That sure needs cleaning up
    css::beans::PropertyValue* pContentVal = xController->getValue(OUString("PrintRange"));
    if( ! pContentVal )
        pContentVal = xController->getValue(OUString("PrintContent"));

    // case 1: UNO API has set "Pages"
    css::beans::PropertyValue* pPagesVal = xController->getValue(OUString("Pages"));
    if( pPagesVal )
    {
        OUString aPagesVal;
        pPagesVal->Value >>= aPagesVal;
        if( !aPagesVal.isEmpty() )
        {
            // "Pages" attribute from API is now equivalent to "PageRange"
            // AND "PrintContent" = 1 except calc where it is "PrintRange" = 1
            // Argh ! That sure needs cleaning up
            if( pContentVal )
            {
                pContentVal->Value = css::uno::makeAny( sal_Int32( 1 ) );
                xController->setValue("PageRange", pPagesVal->Value);
            }
        }
    }
    // case 2: is "All" selected ?
    else if( pContentVal )
    {
        sal_Int32 nContent = -1;
        if( pContentVal->Value >>= nContent )
        {
            if( nContent == 0 )
            {
                // do not overwrite PageRange if it is already set
                css::beans::PropertyValue* pRangeVal = xController->getValue(OUString("PageRange"));
                OUString aRange;
                if( pRangeVal )
                    pRangeVal->Value >>= aRange;
                if( aRange.isEmpty() )
                {
                    sal_Int32 nPages = xController->getPageCount();
                    if( nPages > 0 )
                    {
                        OUStringBuffer aBuf( 32 );
                        aBuf.append( "1" );
                        if( nPages > 1 )
                        {
                            aBuf.append( "-" );
                            aBuf.append( nPages );
                        }
                        xController->setValue("PageRange", css::uno::makeAny(aBuf.makeStringAndClear()));
                    }
                }
            }
        }
    }

    css::beans::PropertyValue* pReverseVal = xController->getValue(OUString("PrintReverse"));
    if( pReverseVal )
    {
        bool bReverse = false;
        pReverseVal->Value >>= bReverse;
        xController->setReversePrint( bReverse );
    }

    css::beans::PropertyValue* pPapersizeFromSetupVal = xController->getValue(OUString("PapersizeFromSetup"));
    if( pPapersizeFromSetupVal )
    {
        bool bPapersizeFromSetup = false;
        pPapersizeFromSetupVal->Value >>= bPapersizeFromSetup;
        xController->setPapersizeFromSetup(bPapersizeFromSetup);
    }

    // setup NUp printing from properties
    sal_Int32 nRows = xController->getIntProperty("NUpRows", 1);
    sal_Int32 nCols = xController->getIntProperty("NUpColumns", 1);
    if( nRows > 1 || nCols > 1 )
    {
        PrinterController::MultiPageSetup aMPS;
        aMPS.nRows         = nRows > 1 ? nRows : 1;
        aMPS.nColumns      = nCols > 1 ? nCols : 1;
        sal_Int32 nValue = xController->getIntProperty("NUpPageMarginLeft", aMPS.nLeftMargin);
        if( nValue >= 0 )
            aMPS.nLeftMargin = nValue;
        nValue = xController->getIntProperty("NUpPageMarginRight", aMPS.nRightMargin);
        if( nValue >= 0 )
            aMPS.nRightMargin = nValue;
        nValue = xController->getIntProperty( "NUpPageMarginTop", aMPS.nTopMargin );
        if( nValue >= 0 )
            aMPS.nTopMargin = nValue;
        nValue = xController->getIntProperty( "NUpPageMarginBottom", aMPS.nBottomMargin );
        if( nValue >= 0 )
            aMPS.nBottomMargin = nValue;
        nValue = xController->getIntProperty( "NUpHorizontalSpacing", aMPS.nHorizontalSpacing );
        if( nValue >= 0 )
            aMPS.nHorizontalSpacing = nValue;
        nValue = xController->getIntProperty( "NUpVerticalSpacing", aMPS.nVerticalSpacing );
        if( nValue >= 0 )
            aMPS.nVerticalSpacing = nValue;
        aMPS.bDrawBorder = xController->getBoolProperty( "NUpDrawBorder", aMPS.bDrawBorder );
        aMPS.nOrder = static_cast<NupOrderType>(xController->getIntProperty( "NUpSubPageOrder", (sal_Int32)aMPS.nOrder ));
        aMPS.aPaperSize = xController->getPrinter()->PixelToLogic( xController->getPrinter()->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) );
        css::beans::PropertyValue* pPgSizeVal = xController->getValue( OUString( "NUpPaperSize" ) );
        css::awt::Size aSizeVal;
        if( pPgSizeVal && (pPgSizeVal->Value >>= aSizeVal) )
        {
            aMPS.aPaperSize.Width() = aSizeVal.Width;
            aMPS.aPaperSize.Height() = aSizeVal.Height;
        }

        xController->setMultipage( aMPS );
    }

    // in direct print case check whether there is anything to print.
    // if not, show an errorbox (if appropriate)
    if( xController->isShowDialogs() && xController->isDirectPrint() )
    {
        if( xController->getFilteredPageCount() == 0 )
        {
            ScopedVclPtrInstance<MessageDialog> aBox(
                nullptr, "ErrorNoContentDialog",
                "vcl/ui/errornocontentdialog.ui");
            aBox->Execute();
            return false;
        }
    }

    // check if the printer brings up its own dialog
    // in that case leave the work to that dialog
    if( ! xController->getPrinter()->GetCapabilities( PrinterCapType::ExternalDialog ) &&
        ! xController->isDirectPrint() &&
        xController->isShowDialogs()
        )
    {
        try
        {
            ScopedVclPtrInstance< PrintDialog > aDlg( nullptr, xController );
            if( ! aDlg->Execute() )
            {
                xController->abortJob();
                return false;
            }
            if( aDlg->isPrintToFile() )
            {
                OUString aFile = queryFile( xController->getPrinter().get() );
                if( aFile.isEmpty() )
                {
                    xController->abortJob();
                    return false;
                }
                xController->setValue( "LocalFileName",
                                       css::uno::makeAny( aFile ) );
            }
            else if( aDlg->isSingleJobs() )
            {
                xController->setValue( "PrintCollateAsSingleJobs",
                                       css::uno::makeAny( true ) );
            }
        }
        catch (const std::bad_alloc&)
        {
        }
    }

    xController->pushPropertiesToPrinter();
    return true;
}

bool Printer::ExecutePrintJob(std::shared_ptr<PrinterController> xController)
{
    OUString aJobName;
    css::beans::PropertyValue* pJobNameVal = xController->getValue( OUString( "JobName" ) );
    if( pJobNameVal )
        pJobNameVal->Value >>= aJobName;

    return xController->getPrinter()->StartJob( aJobName, xController );
}

void Printer::FinishPrintJob(const std::shared_ptr<PrinterController>& xController)
{
    xController->resetPaperToLastConfigured();
    xController->jobFinished( xController->getJobState() );
}

void Printer::ImplPrintJob(const std::shared_ptr<PrinterController>& xController,
                           const JobSetup& i_rInitSetup)
{
    if (PreparePrintJob(xController, i_rInitSetup))
    {
        ExecutePrintJob(xController);
    }
    FinishPrintJob(xController);
}

bool Printer::StartJob( const OUString& i_rJobName, std::shared_ptr<vcl::PrinterController>& i_xController)
{
    mnError = PRINTER_OK;

    if ( IsDisplayPrinter() )
        return false;

    if ( IsJobActive() || IsPrinting() )
        return false;

    sal_uInt32 nCopies = mnCopyCount;
    bool    bCollateCopy = mbCollateCopy;
    bool    bUserCopy = false;

    if ( nCopies > 1 )
    {
        const sal_uInt32 nDevCopy = GetCapabilities( bCollateCopy
            ? PrinterCapType::CollateCopies
            : PrinterCapType::Copies );

        // need to do copies by hand ?
        if ( nCopies > nDevCopy )
        {
            bUserCopy = true;
            nCopies = 1;
            bCollateCopy = false;
        }
    }
    else
        bCollateCopy = false;

    ImplSVData* pSVData = ImplGetSVData();
    mpPrinter = pSVData->mpDefInst->CreatePrinter( mpInfoPrinter );

    if (!mpPrinter)
        return false;

    bool bSinglePrintJobs = false;
    css::beans::PropertyValue* pSingleValue = i_xController->getValue(OUString("PrintCollateAsSingleJobs"));
    if( pSingleValue )
    {
        pSingleValue->Value >>= bSinglePrintJobs;
    }

    css::beans::PropertyValue* pFileValue = i_xController->getValue(OUString("LocalFileName"));
    if( pFileValue )
    {
        OUString aFile;
        pFileValue->Value >>= aFile;
        if( !aFile.isEmpty() )
        {
            mbPrintFile = true;
            maPrintFile = aFile;
            bSinglePrintJobs = false;
        }
    }

    OUString* pPrintFile = nullptr;
    if ( mbPrintFile )
        pPrintFile = &maPrintFile;
    mpPrinterOptions->ReadFromConfig( mbPrintFile );

    maJobName               = i_rJobName;
    mnCurPage               = 1;
    mnCurPrintPage          = 1;
    mbPrinting              = true;
    if( GetCapabilities( PrinterCapType::UsePullModel ) )
    {
        mbJobActive             = true;
        // SAL layer does all necessary page printing
        // and also handles showing a dialog
        // that also means it must call jobStarted when the dialog is finished
        // it also must set the JobState of the Controller
        if( mpPrinter->StartJob( pPrintFile,
                                 i_rJobName,
                                 Application::GetDisplayName(),
                                 &maJobSetup.ImplGetData(),
                                 *i_xController) )
        {
            EndJob();
        }
        else
        {
            mnError = ImplSalPrinterErrorCodeToVCL(mpPrinter->GetErrorCode());
            if ( !mnError )
                mnError = PRINTER_GENERALERROR;
            pSVData->mpDefInst->DestroyPrinter( mpPrinter );
            mnCurPage           = 0;
            mnCurPrintPage      = 0;
            mbPrinting          = false;
            mpPrinter = nullptr;
            mbJobActive = false;

            GDIMetaFile aDummyFile;
            i_xController->setLastPage(true);
            i_xController->getFilteredPageFile(0, aDummyFile);

            return false;
        }
    }
    else
    {
        // possibly a dialog has been shown
        // now the real job starts
        i_xController->setJobState( css::view::PrintableState_JOB_STARTED );
        i_xController->jobStarted();

        int nJobs = 1;
        int nOuterRepeatCount = 1;
        int nInnerRepeatCount = 1;
        if( bUserCopy )
        {
            if( mbCollateCopy )
                nOuterRepeatCount = mnCopyCount;
            else
                nInnerRepeatCount = mnCopyCount;
        }
        if( bSinglePrintJobs )
        {
            nJobs = mnCopyCount;
            nCopies = 1;
            nOuterRepeatCount = nInnerRepeatCount = 1;
        }

        for( int nJobIteration = 0; nJobIteration < nJobs; nJobIteration++ )
        {
            bool bError = false;
            if( mpPrinter->StartJob( pPrintFile,
                                     i_rJobName,
                                     Application::GetDisplayName(),
                                     nCopies,
                                     bCollateCopy,
                                     i_xController->isDirectPrint(),
                                     &maJobSetup.ImplGetData() ) )
            {
                bool bAborted = false;
                mbJobActive             = true;
                i_xController->createProgressDialog();
                const int nPages = i_xController->getFilteredPageCount();
                // abort job, if no pages will be printed.
                if ( nPages == 0 )
                {
                    i_xController->abortJob();
                    bAborted = true;
                }
                for( int nOuterIteration = 0; nOuterIteration < nOuterRepeatCount && ! bAborted; nOuterIteration++ )
                {
                    for( int nPage = 0; nPage < nPages && ! bAborted; nPage++ )
                    {
                        for( int nInnerIteration = 0; nInnerIteration < nInnerRepeatCount && ! bAborted; nInnerIteration++ )
                        {
                            if( nPage == nPages-1 &&
                                nOuterIteration == nOuterRepeatCount-1 &&
                                nInnerIteration == nInnerRepeatCount-1 &&
                                nJobIteration == nJobs-1 )
                            {
                                i_xController->setLastPage(true);
                            }
                            i_xController->printFilteredPage(nPage);
                            if (i_xController->isProgressCanceled())
                            {
                                i_xController->abortJob();
                            }
                            if (i_xController->getJobState() ==
                                    css::view::PrintableState_JOB_ABORTED)
                            {
                                bAborted = true;
                            }
                        }
                    }
                    // FIXME: duplex ?
                }
                EndJob();

                if( nJobIteration < nJobs-1 )
                {
                    mpPrinter = pSVData->mpDefInst->CreatePrinter( mpInfoPrinter );

                    if ( mpPrinter )
                    {
                        maJobName               = i_rJobName;
                        mnCurPage               = 1;
                        mnCurPrintPage          = 1;
                        mbPrinting              = true;
                    }
                    else
                        bError = true;
                }
            }
            else
                bError = true;

            if( bError )
            {
                mnError = mpPrinter ? ImplSalPrinterErrorCodeToVCL(mpPrinter->GetErrorCode()) : 0;
                if ( !mnError )
                    mnError = PRINTER_GENERALERROR;
                i_xController->setJobState( mnError == PRINTER_ABORT
                                            ? css::view::PrintableState_JOB_ABORTED
                                            : css::view::PrintableState_JOB_FAILED );
                if( mpPrinter )
                    pSVData->mpDefInst->DestroyPrinter( mpPrinter );
                mnCurPage           = 0;
                mnCurPrintPage      = 0;
                mbPrinting          = false;
                mpPrinter = nullptr;

                return false;
            }
        }

        if (i_xController->getJobState() == css::view::PrintableState_JOB_STARTED)
            i_xController->setJobState(css::view::PrintableState_JOB_SPOOLED);
    }

    // make last used printer persistent for UI jobs
    if (i_xController->isShowDialogs() && !i_xController->isDirectPrint())
    {
        SettingsConfigItem* pItem = SettingsConfigItem::get();
        pItem->setValue( "PrintDialog",
                         "LastPrinterUsed",
                         GetName()
                         );
    }

    return true;
}

PrinterController::~PrinterController()
{
}

css::view::PrintableState PrinterController::getJobState() const
{
    return mpImplData->meJobState;
}

void PrinterController::setJobState( css::view::PrintableState i_eState )
{
    mpImplData->meJobState = i_eState;
}

const VclPtr<Printer>& PrinterController::getPrinter() const
{
    return mpImplData->mxPrinter;
}

void PrinterController::setPrinter( const VclPtr<Printer>& i_rPrinter )
{
    mpImplData->mxPrinter = i_rPrinter;
    setValue( "Name",
              css::uno::makeAny( OUString( i_rPrinter->GetName() ) ) );
    mpImplData->mnDefaultPaperBin = mpImplData->mxPrinter->GetPaperBin();
    mpImplData->mxPrinter->Push();
    mpImplData->mxPrinter->SetMapMode(MapMode(MapUnit::Map100thMM));
    mpImplData->maDefaultPageSize = mpImplData->mxPrinter->GetPaperSize();
    mpImplData->mxPrinter->Pop();
    mpImplData->mnFixedPaperBin = -1;
}

void PrinterController::resetPrinterOptions( bool i_bFileOutput )
{
    PrinterOptions aOpt;
    aOpt.ReadFromConfig( i_bFileOutput );
    mpImplData->mxPrinter->SetPrinterOptions( aOpt );
}

bool PrinterController::setupPrinter( vcl::Window* i_pParent )
{
    bool bRet = false;
    if( mpImplData->mxPrinter.get() )
    {
        mpImplData->mxPrinter->Push();
        mpImplData->mxPrinter->SetMapMode(MapMode(MapUnit::Map100thMM));

        // get current data
        Size aPaperSize(mpImplData->mxPrinter->GetPaperSize());
        sal_uInt16 nPaperBin = mpImplData->mxPrinter->GetPaperBin();

        // reset paper size back to last configured size, not
        // whatever happens to be the current page
        // (but only if the printer config has changed, otherwise
        // don't override printer page auto-detection - tdf#91362)
        if (getPrinterModified())
        {
            resetPaperToLastConfigured();
        }

        // call driver setup
        bRet = mpImplData->mxPrinter->Setup( i_pParent, getPapersizeFromSetup() );
        Size aNewPaperSize(mpImplData->mxPrinter->GetPaperSize());
        if (bRet)
        {
            bool bInvalidateCache = false;

            // was papersize overridden ? if so we need to take action if we're
            // configured to use the driver papersize
            if (aNewPaperSize != mpImplData->maDefaultPageSize)
            {
                mpImplData->maDefaultPageSize = aNewPaperSize;
                bInvalidateCache = getPapersizeFromSetup();
            }

            // was bin overridden ? if so we need to take action
            sal_uInt16 nNewPaperBin = mpImplData->mxPrinter->GetPaperBin();
            if (nNewPaperBin != nPaperBin)
            {
                mpImplData->mnFixedPaperBin = nNewPaperBin;
                bInvalidateCache = true;
            }

            if (bInvalidateCache)
            {
                mpImplData->maPageCache.invalidate();
            }
        }
        else
        {
            //restore to whatever it was before we entered this method
            if (aPaperSize != aNewPaperSize)
                mpImplData->mxPrinter->SetPaperSizeUser(aPaperSize, !mpImplData->isFixedPageSize());
        }
        mpImplData->mxPrinter->Pop();
    }
    return bRet;
}

PrinterController::PageSize vcl::ImplPrinterControllerData::modifyJobSetup( const css::uno::Sequence< css::beans::PropertyValue >& i_rProps )
{
    PrinterController::PageSize aPageSize;
    aPageSize.aSize = mxPrinter->GetPaperSize();
    css::awt::Size aSetSize, aIsSize;
    sal_Int32 nPaperBin = mnDefaultPaperBin;
    for( sal_Int32 nProperty = 0, nPropertyCount = i_rProps.getLength(); nProperty < nPropertyCount; ++nProperty )
    {
        if ( i_rProps[ nProperty ].Name == "PreferredPageSize" )
        {
            i_rProps[ nProperty ].Value >>= aSetSize;
        }
        else if ( i_rProps[ nProperty ].Name == "PageSize" )
        {
            i_rProps[ nProperty ].Value >>= aIsSize;
        }
        else if ( i_rProps[ nProperty ].Name == "PageIncludesNonprintableArea" )
        {
            bool bVal = false;
            i_rProps[ nProperty ].Value >>= bVal;
            aPageSize.bFullPaper = bVal;
        }
        else if ( i_rProps[ nProperty ].Name == "PrinterPaperTray" )
        {
            sal_Int32 nBin = -1;
            i_rProps[ nProperty ].Value >>= nBin;
            if( nBin >= 0 && nBin < static_cast<sal_Int32>(mxPrinter->GetPaperBinCount()) )
                nPaperBin = nBin;
        }
    }

    Size aCurSize( mxPrinter->GetPaperSize() );
    if( aSetSize.Width && aSetSize.Height )
    {
        Size aSetPaperSize( aSetSize.Width, aSetSize.Height );
        Size aRealPaperSize( getRealPaperSize( aSetPaperSize, true/*bNoNUP*/ ) );
        if( aRealPaperSize != aCurSize )
            aIsSize = aSetSize;
    }

    if( aIsSize.Width && aIsSize.Height )
    {
        aPageSize.aSize.Width() = aIsSize.Width;
        aPageSize.aSize.Height() = aIsSize.Height;

        Size aRealPaperSize( getRealPaperSize( aPageSize.aSize, true/*bNoNUP*/ ) );
        if( aRealPaperSize != aCurSize )
            mxPrinter->SetPaperSizeUser( aRealPaperSize, ! isFixedPageSize() );
    }

    // paper bin set from properties in print dialog overrides
    // application default for a page
    if ( mnFixedPaperBin != -1 )
        nPaperBin = mnFixedPaperBin;

    if( nPaperBin != -1 && nPaperBin != mxPrinter->GetPaperBin() )
        mxPrinter->SetPaperBin( nPaperBin );

    return aPageSize;
}

//fdo#61886

//when printing is finished, set the paper size of the printer to either what
//the user explicitly set as the desired paper size, or fallback to whatever
//the printer had before printing started. That way it doesn't contain the last
//paper size of a multiple paper size using document when we are in our normal
//auto accept document paper size mode and end up overwriting the original
//paper size setting for file->printer_settings just by pressing "ok" in the
//print dialog
void vcl::ImplPrinterControllerData::resetPaperToLastConfigured()
{
    mxPrinter->Push();
    mxPrinter->SetMapMode(MapMode(MapUnit::Map100thMM));
    Size aCurSize(mxPrinter->GetPaperSize());
    if (aCurSize != maDefaultPageSize)
        mxPrinter->SetPaperSizeUser(maDefaultPageSize, !isFixedPageSize());
    mxPrinter->Pop();
}

int PrinterController::getPageCountProtected() const
{
    const MapMode aMapMode( MapUnit::Map100thMM );

    mpImplData->mxPrinter->Push();
    mpImplData->mxPrinter->SetMapMode( aMapMode );
    int nPages = getPageCount();
    mpImplData->mxPrinter->Pop();
    return nPages;
}

css::uno::Sequence< css::beans::PropertyValue > PrinterController::getPageParametersProtected( int i_nPage ) const
{
    const MapMode aMapMode( MapUnit::Map100thMM );

    mpImplData->mxPrinter->Push();
    mpImplData->mxPrinter->SetMapMode( aMapMode );
    css::uno::Sequence< css::beans::PropertyValue > aResult( getPageParameters( i_nPage ) );
    mpImplData->mxPrinter->Pop();
    return aResult;
}

PrinterController::PageSize PrinterController::getPageFile( int i_nUnfilteredPage, GDIMetaFile& o_rMtf, bool i_bMayUseCache )
{
    // update progress if necessary
    if( mpImplData->mpProgress )
    {
        // do nothing if printing is canceled
        if( mpImplData->mpProgress->isCanceled() )
            return PrinterController::PageSize();
        mpImplData->mpProgress->tick();
        Application::Reschedule( true );
    }

    if( i_bMayUseCache )
    {
        PrinterController::PageSize aPageSize;
        if( mpImplData->maPageCache.get( i_nUnfilteredPage, o_rMtf, aPageSize ) )
        {
            return aPageSize;
        }
    }
    else
        mpImplData->maPageCache.invalidate();

    o_rMtf.Clear();

    // get page parameters
    css::uno::Sequence< css::beans::PropertyValue > aPageParm( getPageParametersProtected( i_nUnfilteredPage ) );
    const MapMode aMapMode( MapUnit::Map100thMM );

    mpImplData->mxPrinter->Push();
    mpImplData->mxPrinter->SetMapMode( aMapMode );

    // modify job setup if necessary
    PrinterController::PageSize aPageSize = mpImplData->modifyJobSetup( aPageParm );

    o_rMtf.SetPrefSize( aPageSize.aSize );
    o_rMtf.SetPrefMapMode( aMapMode );

    mpImplData->mxPrinter->EnableOutput( false );

    o_rMtf.Record( mpImplData->mxPrinter.get() );

    printPage( i_nUnfilteredPage );

    o_rMtf.Stop();
    o_rMtf.WindStart();
    mpImplData->mxPrinter->Pop();

    if( i_bMayUseCache )
        mpImplData->maPageCache.insert( i_nUnfilteredPage, o_rMtf, aPageSize );

    // reset "FirstPage" property to false now we've gotten at least our first one
    mpImplData->mbFirstPage = false;

    return aPageSize;
}

static void appendSubPage( GDIMetaFile& o_rMtf, const Rectangle& i_rClipRect, GDIMetaFile& io_rSubPage, bool i_bDrawBorder )
{
    // intersect all clipregion actions with our clip rect
    io_rSubPage.WindStart();
    io_rSubPage.Clip( i_rClipRect );

    // save gstate
    o_rMtf.AddAction( new MetaPushAction( PushFlags::ALL ) );

    // clip to page rect
    o_rMtf.AddAction( new MetaClipRegionAction( vcl::Region( i_rClipRect ), true ) );

    // append the subpage
    io_rSubPage.WindStart();
    io_rSubPage.Play( o_rMtf );

    // restore gstate
    o_rMtf.AddAction( new MetaPopAction() );

    // draw a border
    if( i_bDrawBorder )
    {
        // save gstate
        o_rMtf.AddAction( new MetaPushAction( PushFlags::LINECOLOR | PushFlags::FILLCOLOR | PushFlags::CLIPREGION | PushFlags::MAPMODE ) );
        o_rMtf.AddAction( new MetaMapModeAction( MapMode( MapUnit::Map100thMM ) ) );

        Rectangle aBorderRect( i_rClipRect );
        o_rMtf.AddAction( new MetaLineColorAction( Color( COL_BLACK ), true ) );
        o_rMtf.AddAction( new MetaFillColorAction( Color( COL_TRANSPARENT ), false ) );
        o_rMtf.AddAction( new MetaRectAction( aBorderRect ) );

        // restore gstate
        o_rMtf.AddAction( new MetaPopAction() );
    }
}

PrinterController::PageSize PrinterController::getFilteredPageFile( int i_nFilteredPage, GDIMetaFile& o_rMtf, bool i_bMayUseCache )
{
    const MultiPageSetup& rMPS( mpImplData->maMultiPage );
    int nSubPages = rMPS.nRows * rMPS.nColumns;
    if( nSubPages < 1 )
        nSubPages = 1;

    // reverse sheet order
    if( mpImplData->mbReversePageOrder )
    {
        int nDocPages = getFilteredPageCount();
        i_nFilteredPage = nDocPages - 1 - i_nFilteredPage;
    }

    // there is no filtering to be done (and possibly the page size of the
    // original page is to be set), when N-Up is "neutral" that is there is
    // only one subpage and the margins are 0
    if( nSubPages == 1 &&
        rMPS.nLeftMargin == 0 && rMPS.nRightMargin == 0 &&
        rMPS.nTopMargin == 0 && rMPS.nBottomMargin == 0 )
    {
        PrinterController::PageSize aPageSize = getPageFile( i_nFilteredPage, o_rMtf, i_bMayUseCache );
        if (mpImplData->meJobState != css::view::PrintableState_JOB_STARTED)
        {   // rhbz#657394: check that we are still printing...
            return PrinterController::PageSize();
        }
        Size aPaperSize = mpImplData->getRealPaperSize( aPageSize.aSize, true );
        mpImplData->mxPrinter->SetMapMode( MapMode( MapUnit::Map100thMM ) );
        mpImplData->mxPrinter->SetPaperSizeUser( aPaperSize, ! mpImplData->isFixedPageSize() );
        if( aPaperSize != aPageSize.aSize )
        {
            // user overridden page size, center Metafile
            o_rMtf.WindStart();
            long nDX = (aPaperSize.Width() - aPageSize.aSize.Width()) / 2;
            long nDY = (aPaperSize.Height() - aPageSize.aSize.Height()) / 2;
            o_rMtf.Move( nDX, nDY, mpImplData->mxPrinter->GetDPIX(), mpImplData->mxPrinter->GetDPIY() );
            o_rMtf.WindStart();
            o_rMtf.SetPrefSize( aPaperSize );
            aPageSize.aSize = aPaperSize;
        }
        return aPageSize;
    }

    // set last page property really only on the very last page to be rendered
    // that is on the last subpage of a NUp run
    bool bIsLastPage = mpImplData->mbLastPage;
    mpImplData->mbLastPage = false;

    Size aPaperSize( mpImplData->getRealPaperSize( mpImplData->maMultiPage.aPaperSize, false ) );

    // multi page area: page size minus margins + one time spacing right and down
    // the added spacing is so each subpage can be calculated including its spacing
    Size aMPArea( aPaperSize );
    aMPArea.Width()  -= rMPS.nLeftMargin + rMPS.nRightMargin;
    aMPArea.Width()  += rMPS.nHorizontalSpacing;
    aMPArea.Height() -= rMPS.nTopMargin + rMPS.nBottomMargin;
    aMPArea.Height() += rMPS.nVerticalSpacing;

    // determine offsets
    long nAdvX = aMPArea.Width() / rMPS.nColumns;
    long nAdvY = aMPArea.Height() / rMPS.nRows;

    // determine size of a "cell" subpage, leave a little space around pages
    Size aSubPageSize( nAdvX - rMPS.nHorizontalSpacing, nAdvY - rMPS.nVerticalSpacing );

    o_rMtf.Clear();
    o_rMtf.SetPrefSize( aPaperSize );
    o_rMtf.SetPrefMapMode( MapMode( MapUnit::Map100thMM ) );
    o_rMtf.AddAction( new MetaMapModeAction( MapMode( MapUnit::Map100thMM ) ) );

    int nDocPages = getPageCountProtected();
    if (mpImplData->meJobState != css::view::PrintableState_JOB_STARTED)
    {   // rhbz#657394: check that we are still printing...
        return PrinterController::PageSize();
    }
    for( int nSubPage = 0; nSubPage < nSubPages; nSubPage++ )
    {
        // map current sub page to real page
        int nPage = i_nFilteredPage * nSubPages + nSubPage;
        if( nSubPage == nSubPages-1 ||
            nPage == nDocPages-1 )
        {
            mpImplData->mbLastPage = bIsLastPage;
        }
        if( nPage >= 0 && nPage < nDocPages )
        {
            GDIMetaFile aPageFile;
            PrinterController::PageSize aPageSize = getPageFile( nPage, aPageFile, i_bMayUseCache );
            if( aPageSize.aSize.Width() && aPageSize.aSize.Height() )
            {
                long nCellX = 0, nCellY = 0;
                switch( rMPS.nOrder )
                {
                case NupOrderType::LRTB:
                    nCellX = (nSubPage % rMPS.nColumns);
                    nCellY = (nSubPage / rMPS.nColumns);
                    break;
                case NupOrderType::TBLR:
                    nCellX = (nSubPage / rMPS.nRows);
                    nCellY = (nSubPage % rMPS.nRows);
                    break;
                case NupOrderType::RLTB:
                    nCellX = rMPS.nColumns - 1 - (nSubPage % rMPS.nColumns);
                    nCellY = (nSubPage / rMPS.nColumns);
                    break;
                case NupOrderType::TBRL:
                    nCellX = rMPS.nColumns - 1 - (nSubPage / rMPS.nRows);
                    nCellY = (nSubPage % rMPS.nRows);
                    break;
                }
                // scale the metafile down to a sub page size
                double fScaleX = double(aSubPageSize.Width())/double(aPageSize.aSize.Width());
                double fScaleY = double(aSubPageSize.Height())/double(aPageSize.aSize.Height());
                double fScale  = std::min( fScaleX, fScaleY );
                aPageFile.Scale( fScale, fScale );
                aPageFile.WindStart();

                // move the subpage so it is centered in its "cell"
                long nOffX = (aSubPageSize.Width() - long(double(aPageSize.aSize.Width()) * fScale)) / 2;
                long nOffY = (aSubPageSize.Height() - long(double(aPageSize.aSize.Height()) * fScale)) / 2;
                long nX = rMPS.nLeftMargin + nOffX + nAdvX * nCellX;
                long nY = rMPS.nTopMargin + nOffY + nAdvY * nCellY;
                aPageFile.Move( nX, nY, mpImplData->mxPrinter->GetDPIX(), mpImplData->mxPrinter->GetDPIY() );
                aPageFile.WindStart();
                // calculate border rectangle
                Rectangle aSubPageRect( Point( nX, nY ),
                                        Size( long(double(aPageSize.aSize.Width())*fScale),
                                              long(double(aPageSize.aSize.Height())*fScale) ) );

                // append subpage to page
                appendSubPage( o_rMtf, aSubPageRect, aPageFile, rMPS.bDrawBorder );
            }
        }
    }
    o_rMtf.WindStart();

    // subsequent getPageFile calls have changed the paper, reset it to current value
    mpImplData->mxPrinter->SetMapMode( MapMode( MapUnit::Map100thMM ) );
    mpImplData->mxPrinter->SetPaperSizeUser( aPaperSize, ! mpImplData->isFixedPageSize() );

    return PrinterController::PageSize( aPaperSize, true );
}

int PrinterController::getFilteredPageCount()
{
    int nDiv = mpImplData->maMultiPage.nRows * mpImplData->maMultiPage.nColumns;
    if( nDiv < 1 )
        nDiv = 1;
    return (getPageCountProtected() + (nDiv-1)) / nDiv;
}

DrawModeFlags PrinterController::removeTransparencies( GDIMetaFile& i_rIn, GDIMetaFile& o_rOut )
{
    DrawModeFlags nRestoreDrawMode = mpImplData->mxPrinter->GetDrawMode();
    sal_Int32 nMaxBmpDPIX = mpImplData->mxPrinter->GetDPIX();
    sal_Int32 nMaxBmpDPIY = mpImplData->mxPrinter->GetDPIY();

    const PrinterOptions&   rPrinterOptions = mpImplData->mxPrinter->GetPrinterOptions();

    static const sal_Int32 OPTIMAL_BMP_RESOLUTION = 300;
    static const sal_Int32 NORMAL_BMP_RESOLUTION  = 200;

    if( rPrinterOptions.IsReduceBitmaps() )
    {
        // calculate maximum resolution for bitmap graphics
        if( PrinterBitmapMode::Optimal == rPrinterOptions.GetReducedBitmapMode() )
        {
            nMaxBmpDPIX = std::min( sal_Int32(OPTIMAL_BMP_RESOLUTION), nMaxBmpDPIX );
            nMaxBmpDPIY = std::min( sal_Int32(OPTIMAL_BMP_RESOLUTION), nMaxBmpDPIY );
        }
        else if( PrinterBitmapMode::Normal == rPrinterOptions.GetReducedBitmapMode() )
        {
            nMaxBmpDPIX = std::min( sal_Int32(NORMAL_BMP_RESOLUTION), nMaxBmpDPIX );
            nMaxBmpDPIY = std::min( sal_Int32(NORMAL_BMP_RESOLUTION), nMaxBmpDPIY );
        }
        else
        {
            nMaxBmpDPIX = std::min( sal_Int32(rPrinterOptions.GetReducedBitmapResolution()), nMaxBmpDPIX );
            nMaxBmpDPIY = std::min( sal_Int32(rPrinterOptions.GetReducedBitmapResolution()), nMaxBmpDPIY );
        }
    }

    // convert to greyscales
    if( rPrinterOptions.IsConvertToGreyscales() )
    {
        mpImplData->mxPrinter->SetDrawMode( mpImplData->mxPrinter->GetDrawMode() |
                                            ( DrawModeFlags::GrayLine | DrawModeFlags::GrayFill | DrawModeFlags::GrayText |
                                              DrawModeFlags::GrayBitmap | DrawModeFlags::GrayGradient ) );
    }

    // disable transparency output
    if( rPrinterOptions.IsReduceTransparency() && ( PrinterTransparencyMode::NONE == rPrinterOptions.GetReducedTransparencyMode() ) )
    {
        mpImplData->mxPrinter->SetDrawMode( mpImplData->mxPrinter->GetDrawMode() | DrawModeFlags::NoTransparency );
    }

    Color aBg( COL_TRANSPARENT ); // default: let RemoveTransparenciesFromMetaFile do its own background logic
    if( mpImplData->maMultiPage.nRows * mpImplData->maMultiPage.nColumns > 1 )
    {
        // in N-Up printing we have no "page" background operation
        // we also have no way to determine the paper color
        // so let's go for white, which will kill 99.9% of the real cases
        aBg = Color( COL_WHITE );
    }
    mpImplData->mxPrinter->RemoveTransparenciesFromMetaFile( i_rIn, o_rOut, nMaxBmpDPIX, nMaxBmpDPIY,
                                                             rPrinterOptions.IsReduceTransparency(),
                                                             rPrinterOptions.GetReducedTransparencyMode() == PrinterTransparencyMode::Auto,
                                                             rPrinterOptions.IsReduceBitmaps() && rPrinterOptions.IsReducedBitmapIncludesTransparency(),
                                                             aBg
                                                             );
    return nRestoreDrawMode;
}

void PrinterController::printFilteredPage( int i_nPage )
{
    if( mpImplData->meJobState != css::view::PrintableState_JOB_STARTED )
        return;

    GDIMetaFile aPageFile;
    PrinterController::PageSize aPageSize = getFilteredPageFile( i_nPage, aPageFile );

    if (mpImplData->meJobState != css::view::PrintableState_JOB_STARTED)
    {   // rhbz#657394: check that we are still printing...
        return;
    }

    if( mpImplData->mpProgress )
    {
        // do nothing if printing is canceled
        if( mpImplData->mpProgress->isCanceled() )
        {
            setJobState( css::view::PrintableState_JOB_ABORTED );
            return;
        }
    }

    // in N-Up printing set the correct page size
    mpImplData->mxPrinter->SetMapMode( MapUnit::Map100thMM );
    // aPageSize was filtered through mpImplData->getRealPaperSize already by getFilteredPageFile()
    mpImplData->mxPrinter->SetPaperSizeUser( aPageSize.aSize, ! mpImplData->isFixedPageSize() );
    if( mpImplData->mnFixedPaperBin != -1 &&
        mpImplData->mxPrinter->GetPaperBin() != mpImplData->mnFixedPaperBin )
    {
        mpImplData->mxPrinter->SetPaperBin( mpImplData->mnFixedPaperBin );
    }

    // if full paper is meant to be used, move the output to accommodate for pageoffset
    if( aPageSize.bFullPaper )
    {
        Point aPageOffset( mpImplData->mxPrinter->GetPageOffset() );
        aPageFile.WindStart();
        aPageFile.Move( -aPageOffset.X(), -aPageOffset.Y(), mpImplData->mxPrinter->GetDPIX(), mpImplData->mxPrinter->GetDPIY() );
    }

    GDIMetaFile aCleanedFile;
    DrawModeFlags nRestoreDrawMode = removeTransparencies( aPageFile, aCleanedFile );

    mpImplData->mxPrinter->EnableOutput();

    // actually print the page
    mpImplData->mxPrinter->ImplStartPage();

    mpImplData->mxPrinter->Push();
    aCleanedFile.WindStart();
    aCleanedFile.Play( mpImplData->mxPrinter.get() );
    mpImplData->mxPrinter->Pop();

    mpImplData->mxPrinter->ImplEndPage();

    mpImplData->mxPrinter->SetDrawMode( nRestoreDrawMode );
}

void PrinterController::jobStarted()
{
}

void PrinterController::jobFinished( css::view::PrintableState )
{
}

void PrinterController::abortJob()
{
    setJobState( css::view::PrintableState_JOB_ABORTED );
    // applications (well, sw) depend on a page request with "IsLastPage" = true
    // to free resources, else they (well, sw) will crash eventually
    setLastPage( true );
    mpImplData->mpProgress.disposeAndClear();
    GDIMetaFile aMtf;
    getPageFile( 0, aMtf );
}

void PrinterController::setLastPage( bool i_bLastPage )
{
    mpImplData->mbLastPage = i_bLastPage;
}

void PrinterController::setReversePrint( bool i_bReverse )
{
    mpImplData->mbReversePageOrder = i_bReverse;
}

bool PrinterController::getReversePrint() const
{
    return mpImplData->mbReversePageOrder;
}

void PrinterController::setPapersizeFromSetup( bool i_bPapersizeFromSetup )
{
    mpImplData->mbPapersizeFromSetup = i_bPapersizeFromSetup;
}

bool PrinterController::getPapersizeFromSetup() const
{
    return mpImplData->mbPapersizeFromSetup;
}

void PrinterController::setPrinterModified( bool i_bPrinterModified )
{
    mpImplData->mbPrinterModified = i_bPrinterModified;
}

bool PrinterController::getPrinterModified() const
{
    return mpImplData->mbPrinterModified;
}

css::uno::Sequence< css::beans::PropertyValue > PrinterController::getJobProperties( const css::uno::Sequence< css::beans::PropertyValue >& i_rMergeList ) const
{
    std::unordered_set< OUString, OUStringHash > aMergeSet;
    size_t nResultLen = size_t(i_rMergeList.getLength()) + mpImplData->maUIProperties.size() + 3;
    for( int i = 0; i < i_rMergeList.getLength(); i++ )
        aMergeSet.insert( i_rMergeList[i].Name );

    css::uno::Sequence< css::beans::PropertyValue > aResult( nResultLen );
    for( int i = 0; i < i_rMergeList.getLength(); i++ )
        aResult[i] = i_rMergeList[i];
    int nCur = i_rMergeList.getLength();
    for(css::beans::PropertyValue & rPropVal : mpImplData->maUIProperties)
    {
        if( aMergeSet.find( rPropVal.Name ) == aMergeSet.end() )
            aResult[nCur++] = rPropVal;
    }
    // append IsFirstPage
    if( aMergeSet.find( OUString( "IsFirstPage" ) ) == aMergeSet.end() )
    {
        css::beans::PropertyValue aVal;
        aVal.Name = "IsFirstPage";
        aVal.Value <<= mpImplData->mbFirstPage;
        aResult[nCur++] = aVal;
    }
    // append IsLastPage
    if( aMergeSet.find( OUString( "IsLastPage" ) ) == aMergeSet.end() )
    {
        css::beans::PropertyValue aVal;
        aVal.Name = "IsLastPage";
        aVal.Value <<= mpImplData->mbLastPage;
        aResult[nCur++] = aVal;
    }
    // append IsPrinter
    if( aMergeSet.find( OUString( "IsPrinter" ) ) == aMergeSet.end() )
    {
        css::beans::PropertyValue aVal;
        aVal.Name = "IsPrinter";
        aVal.Value <<= true;
        aResult[nCur++] = aVal;
    }
    aResult.realloc( nCur );
    return aResult;
}

const css::uno::Sequence< css::beans::PropertyValue >& PrinterController::getUIOptions() const
{
    return mpImplData->maUIOptions;
}

css::beans::PropertyValue* PrinterController::getValue( const OUString& i_rProperty )
{
    std::unordered_map< OUString, size_t, OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    return it != mpImplData->maPropertyToIndex.end() ? &mpImplData->maUIProperties[it->second] : nullptr;
}

const css::beans::PropertyValue* PrinterController::getValue( const OUString& i_rProperty ) const
{
    std::unordered_map< OUString, size_t, OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    return it != mpImplData->maPropertyToIndex.end() ? &mpImplData->maUIProperties[it->second] : nullptr;
}

void PrinterController::setValue( const OUString& i_rPropertyName, const css::uno::Any& i_rValue )
{
    css::beans::PropertyValue aVal;
    aVal.Name = i_rPropertyName;
    aVal.Value = i_rValue;

    setValue( aVal );
}

void PrinterController::setValue( const css::beans::PropertyValue& i_rPropertyValue )
{
    std::unordered_map< OUString, size_t, OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rPropertyValue.Name );
    if( it != mpImplData->maPropertyToIndex.end() )
        mpImplData->maUIProperties[ it->second ] = i_rPropertyValue;
    else
    {
        // insert correct index into property map
        mpImplData->maPropertyToIndex[ i_rPropertyValue.Name ] = mpImplData->maUIProperties.size();
        mpImplData->maUIProperties.push_back( i_rPropertyValue );
        mpImplData->maUIPropertyEnabled.push_back( true );
    }
}

void PrinterController::setUIOptions( const css::uno::Sequence< css::beans::PropertyValue >& i_rOptions )
{
    SAL_WARN_IF( mpImplData->maUIOptions.getLength() != 0, "vcl.gdi", "setUIOptions called twice !" );

    mpImplData->maUIOptions = i_rOptions;

    for( int i = 0; i < i_rOptions.getLength(); i++ )
    {
        css::uno::Sequence< css::beans::PropertyValue > aOptProp;
        i_rOptions[i].Value >>= aOptProp;
        bool bIsEnabled = true;
        bool bHaveProperty = false;
        OUString aPropName;
        vcl::ImplPrinterControllerData::ControlDependency aDep;
        css::uno::Sequence< sal_Bool > aChoicesDisabled;
        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const css::beans::PropertyValue& rEntry( aOptProp[ n ] );
            if ( rEntry.Name == "Property" )
            {
                css::beans::PropertyValue aVal;
                rEntry.Value >>= aVal;
                DBG_ASSERT( mpImplData->maPropertyToIndex.find( aVal.Name )
                            == mpImplData->maPropertyToIndex.end(), "duplicate property entry" );
                setValue( aVal );
                aPropName = aVal.Name;
                bHaveProperty = true;
            }
            else if ( rEntry.Name == "Enabled" )
            {
                bool bValue = true;
                rEntry.Value >>= bValue;
                bIsEnabled = bValue;
            }
            else if ( rEntry.Name == "DependsOnName" )
            {
                rEntry.Value >>= aDep.maDependsOnName;
            }
            else if ( rEntry.Name == "DependsOnEntry" )
            {
                rEntry.Value >>= aDep.mnDependsOnEntry;
            }
            else if ( rEntry.Name == "ChoicesDisabled" )
            {
                rEntry.Value >>= aChoicesDisabled;
            }
        }
        if( bHaveProperty )
        {
            vcl::ImplPrinterControllerData::PropertyToIndexMap::const_iterator it =
                mpImplData->maPropertyToIndex.find( aPropName );
            // sanity check
            if( it != mpImplData->maPropertyToIndex.end() )
            {
                mpImplData->maUIPropertyEnabled[ it->second ] = bIsEnabled;
            }
            if( !aDep.maDependsOnName.isEmpty() )
                mpImplData->maControlDependencies[ aPropName ] = aDep;
            if( aChoicesDisabled.getLength() > 0 )
                mpImplData->maChoiceDisableMap[ aPropName ] = aChoicesDisabled;
        }
    }
}

bool PrinterController::isUIOptionEnabled( const OUString& i_rProperty ) const
{
    bool bEnabled = false;
    std::unordered_map< OUString, size_t, OUStringHash >::const_iterator prop_it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    if( prop_it != mpImplData->maPropertyToIndex.end() )
    {
        bEnabled = mpImplData->maUIPropertyEnabled[prop_it->second];

        if( bEnabled )
        {
            // check control dependencies
            vcl::ImplPrinterControllerData::ControlDependencyMap::const_iterator it =
                mpImplData->maControlDependencies.find( i_rProperty );
            if( it != mpImplData->maControlDependencies.end() )
            {
                // check if the dependency is enabled
                // if the dependency is disabled, we are too
                bEnabled = isUIOptionEnabled( it->second.maDependsOnName );

                if( bEnabled )
                {
                    // does the dependency have the correct value ?
                    const css::beans::PropertyValue* pVal = getValue( it->second.maDependsOnName );
                    OSL_ENSURE( pVal, "unknown property in dependency" );
                    if( pVal )
                    {
                        sal_Int32 nDepVal = 0;
                        bool bDepVal = false;
                        if( pVal->Value >>= nDepVal )
                        {
                            bEnabled = (nDepVal == it->second.mnDependsOnEntry) || (it->second.mnDependsOnEntry == -1);
                        }
                        else if( pVal->Value >>= bDepVal )
                        {
                            // could be a dependency on a checked boolean
                            // in this case the dependency is on a non zero for checked value
                            bEnabled = (   bDepVal && it->second.mnDependsOnEntry != 0) ||
                                       ( ! bDepVal && it->second.mnDependsOnEntry == 0);
                        }
                        else
                        {
                            // if the type does not match something is awry
                            OSL_FAIL( "strange type in control dependency" );
                            bEnabled = false;
                        }
                    }
                }
            }
        }
    }
    return bEnabled;
}

bool PrinterController::isUIChoiceEnabled( const OUString& i_rProperty, sal_Int32 i_nValue ) const
{
    bool bEnabled = true;
    ImplPrinterControllerData::ChoiceDisableMap::const_iterator it =
        mpImplData->maChoiceDisableMap.find( i_rProperty );
    if(it != mpImplData->maChoiceDisableMap.end() )
    {
        const css::uno::Sequence< sal_Bool >& rDisabled( it->second );
        if( i_nValue >= 0 && i_nValue < rDisabled.getLength() )
            bEnabled = ! rDisabled[i_nValue];
    }
    return bEnabled;
}

OUString PrinterController::getDependency( const OUString& i_rProperty ) const
{
    OUString aDependency;

    vcl::ImplPrinterControllerData::ControlDependencyMap::const_iterator it =
        mpImplData->maControlDependencies.find( i_rProperty );
    if( it != mpImplData->maControlDependencies.end() )
        aDependency = it->second.maDependsOnName;

    return aDependency;
}

OUString PrinterController::makeEnabled( const OUString& i_rProperty )
{
    OUString aDependency;

    vcl::ImplPrinterControllerData::ControlDependencyMap::const_iterator it =
        mpImplData->maControlDependencies.find( i_rProperty );
    if( it != mpImplData->maControlDependencies.end() )
    {
        if( isUIOptionEnabled( it->second.maDependsOnName ) )
        {
           aDependency = it->second.maDependsOnName;
           const css::beans::PropertyValue* pVal = getValue( aDependency );
           OSL_ENSURE( pVal, "unknown property in dependency" );
           if( pVal )
           {
               sal_Int32 nDepVal = 0;
               bool bDepVal = false;
               if( pVal->Value >>= nDepVal )
               {
                   if( it->second.mnDependsOnEntry != -1 )
                   {
                       setValue( aDependency, css::uno::makeAny( sal_Int32( it->second.mnDependsOnEntry ) ) );
                   }
               }
               else if( pVal->Value >>= bDepVal )
               {
                   setValue( aDependency, css::uno::makeAny( it->second.mnDependsOnEntry != 0 ) );
               }
               else
               {
                   // if the type does not match something is awry
                   OSL_FAIL( "strange type in control dependency" );
               }
           }
        }
    }

    return aDependency;
}

void PrinterController::createProgressDialog()
{
    if( ! mpImplData->mpProgress )
    {
        bool bShow = true;
        css::beans::PropertyValue* pMonitor = getValue( OUString( "MonitorVisible" ) );
        if( pMonitor )
            pMonitor->Value >>= bShow;
        else
        {
            const css::beans::PropertyValue* pVal = getValue( OUString( "IsApi" ) );
            if( pVal )
            {
                bool bApi = false;
                pVal->Value >>= bApi;
                bShow = ! bApi;
            }
        }

        if( bShow && ! Application::IsHeadlessModeEnabled() )
        {
            mpImplData->mpProgress = VclPtr<PrintProgressDialog>::Create( nullptr, getPageCountProtected() );
            mpImplData->mpProgress->Show();
        }
    }
    else
        mpImplData->mpProgress->reset();
}

bool PrinterController::isProgressCanceled() const
{
    return mpImplData->mpProgress && mpImplData->mpProgress->isCanceled();
}

void PrinterController::setMultipage( const MultiPageSetup& i_rMPS )
{
    mpImplData->maMultiPage = i_rMPS;
}

const PrinterController::MultiPageSetup& PrinterController::getMultipage() const
{
    return mpImplData->maMultiPage;
}

void PrinterController::resetPaperToLastConfigured()
{
    mpImplData->resetPaperToLastConfigured();
}

void PrinterController::pushPropertiesToPrinter()
{
    sal_Int32 nCopyCount = 1;
    // set copycount and collate
    const css::beans::PropertyValue* pVal = getValue( OUString( "CopyCount" ) );
    if( pVal )
        pVal->Value >>= nCopyCount;
    bool bCollate = false;
    pVal = getValue( OUString( "Collate" ) );
    if( pVal )
        pVal->Value >>= bCollate;
    mpImplData->mxPrinter->SetCopyCount( static_cast<sal_uInt16>(nCopyCount), bCollate );

    // duplex mode
    pVal = getValue( OUString( "DuplexMode" ) );
    if( pVal )
    {
        sal_Int16 nDuplex = css::view::DuplexMode::UNKNOWN;
        pVal->Value >>= nDuplex;
        switch( nDuplex )
        {
            case css::view::DuplexMode::OFF: mpImplData->mxPrinter->SetDuplexMode( DuplexMode::Off ); break;
            case css::view::DuplexMode::LONGEDGE: mpImplData->mxPrinter->SetDuplexMode( DuplexMode::LongEdge ); break;
            case css::view::DuplexMode::SHORTEDGE: mpImplData->mxPrinter->SetDuplexMode( DuplexMode::ShortEdge ); break;
        }
    }
}

bool PrinterController::isShowDialogs() const
{
    bool bApi = getBoolProperty( "IsApi", false );
    return ! bApi && ! Application::IsHeadlessModeEnabled();
}

bool PrinterController::isDirectPrint() const
{
    bool bDirect = getBoolProperty( "IsDirect", false );
    return bDirect;
}

bool PrinterController::getBoolProperty( const OUString& i_rProperty, bool i_bFallback ) const
{
    bool bRet = i_bFallback;
    const css::beans::PropertyValue* pVal = getValue( i_rProperty );
    if( pVal )
        pVal->Value >>= bRet;
    return bRet;
}

sal_Int32 PrinterController::getIntProperty( const OUString& i_rProperty, sal_Int32 i_nFallback ) const
{
    sal_Int32 nRet = i_nFallback;
    const css::beans::PropertyValue* pVal = getValue( i_rProperty );
    if( pVal )
        pVal->Value >>= nRet;
    return nRet;
}

/*
 * PrinterOptionsHelper
**/
css::uno::Any PrinterOptionsHelper::getValue( const OUString& i_rPropertyName ) const
{
    css::uno::Any aRet;
    std::unordered_map< OUString, css::uno::Any, OUStringHash >::const_iterator it =
        m_aPropertyMap.find( i_rPropertyName );
    if( it != m_aPropertyMap.end() )
        aRet = it->second;
    return aRet;
}

bool PrinterOptionsHelper::getBoolValue( const OUString& i_rPropertyName, bool i_bDefault ) const
{
    bool bRet = false;
    css::uno::Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= bRet) ? bRet : i_bDefault;
}

sal_Int64 PrinterOptionsHelper::getIntValue( const OUString& i_rPropertyName, sal_Int64 i_nDefault ) const
{
    sal_Int64 nRet = 0;
    css::uno::Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= nRet) ? nRet : i_nDefault;
}

OUString PrinterOptionsHelper::getStringValue( const OUString& i_rPropertyName ) const
{
    OUString aRet;
    css::uno::Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= aRet) ? aRet : OUString();
}

bool PrinterOptionsHelper::processProperties( const css::uno::Sequence< css::beans::PropertyValue >& i_rNewProp )
{
    bool bChanged = false;

    sal_Int32 nElements = i_rNewProp.getLength();
    const css::beans::PropertyValue* pVals = i_rNewProp.getConstArray();
    for( sal_Int32 i = 0; i < nElements; i++ )
    {
        bool bElementChanged = false;
        std::unordered_map< OUString, css::uno::Any, OUStringHash >::iterator it =
            m_aPropertyMap.find( pVals[ i ].Name );
        if( it != m_aPropertyMap.end() )
        {
            if( it->second != pVals[ i ].Value )
                bElementChanged = true;
        }
        else
            bElementChanged = true;

        if( bElementChanged )
        {
            m_aPropertyMap[ pVals[i].Name ] = pVals[i].Value;
            bChanged = true;
        }
    }
    return bChanged;
}

void PrinterOptionsHelper::appendPrintUIOptions( css::uno::Sequence< css::beans::PropertyValue >& io_rProps ) const
{
    if( !m_aUIProperties.empty() )
    {
        sal_Int32 nIndex = io_rProps.getLength();
        io_rProps.realloc( nIndex+1 );
        css::beans::PropertyValue aVal;
        aVal.Name = "ExtraPrintUIOptions";
        aVal.Value = css::uno::makeAny( comphelper::containerToSequence(m_aUIProperties) );
        io_rProps[ nIndex ] = aVal;
    }
}

css::uno::Any PrinterOptionsHelper::setUIControlOpt(const css::uno::Sequence< OUString >& i_rIDs,
                                          const OUString& i_rTitle,
                                          const css::uno::Sequence< OUString >& i_rHelpIds,
                                          const OUString& i_rType,
                                          const css::beans::PropertyValue* i_pVal,
                                          const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    sal_Int32 nElements =
        2                                                             // ControlType + ID
        + (i_rTitle.isEmpty() ? 0 : 1)                                // Text
        + (i_rHelpIds.getLength() ? 1 : 0)                            // HelpId
        + (i_pVal ? 1 : 0)                                            // Property
        + i_rControlOptions.maAddProps.size()                         // additional props
        + (i_rControlOptions.maGroupHint.isEmpty() ? 0 : 1)           // grouping
        + (i_rControlOptions.mbInternalOnly ? 1 : 0)                  // internal hint
        + (i_rControlOptions.mbEnabled ? 0 : 1)                       // enabled
        ;
    if( !i_rControlOptions.maDependsOnName.isEmpty() )
    {
        nElements += 1;
        if( i_rControlOptions.mnDependsOnEntry != -1 )
            nElements += 1;
        if( i_rControlOptions.mbAttachToDependency )
            nElements += 1;
    }

    css::uno::Sequence< css::beans::PropertyValue > aCtrl( nElements );
    sal_Int32 nUsed = 0;
    if( !i_rTitle.isEmpty() )
    {
        aCtrl[nUsed  ].Name  = "Text";
        aCtrl[nUsed++].Value = css::uno::makeAny( i_rTitle );
    }
    if( i_rHelpIds.getLength() )
    {
        aCtrl[nUsed  ].Name = "HelpId";
        aCtrl[nUsed++].Value = css::uno::makeAny( i_rHelpIds );
    }
    aCtrl[nUsed  ].Name  = "ControlType";
    aCtrl[nUsed++].Value = css::uno::makeAny( i_rType );
    aCtrl[nUsed  ].Name  = "ID";
    aCtrl[nUsed++].Value = css::uno::makeAny( i_rIDs );
    if( i_pVal )
    {
        aCtrl[nUsed  ].Name  = "Property";
        aCtrl[nUsed++].Value = css::uno::makeAny( *i_pVal );
    }
    if( !i_rControlOptions.maDependsOnName.isEmpty() )
    {
        aCtrl[nUsed  ].Name  = "DependsOnName";
        aCtrl[nUsed++].Value = css::uno::makeAny( i_rControlOptions.maDependsOnName );
        if( i_rControlOptions.mnDependsOnEntry != -1 )
        {
            aCtrl[nUsed  ].Name  = "DependsOnEntry";
            aCtrl[nUsed++].Value = css::uno::makeAny( i_rControlOptions.mnDependsOnEntry );
        }
        if( i_rControlOptions.mbAttachToDependency )
        {
            aCtrl[nUsed  ].Name  = "AttachToDependency";
            aCtrl[nUsed++].Value = css::uno::makeAny( i_rControlOptions.mbAttachToDependency );
        }
    }
    if( !i_rControlOptions.maGroupHint.isEmpty() )
    {
        aCtrl[nUsed  ].Name    = "GroupingHint";
        aCtrl[nUsed++].Value <<= i_rControlOptions.maGroupHint;
    }
    if( i_rControlOptions.mbInternalOnly )
    {
        aCtrl[nUsed  ].Name    = "InternalUIOnly";
        aCtrl[nUsed++].Value <<= true;
    }
    if( ! i_rControlOptions.mbEnabled )
    {
        aCtrl[nUsed  ].Name    = "Enabled";
        aCtrl[nUsed++].Value <<= false;
    }

    sal_Int32 nAddProps = i_rControlOptions.maAddProps.size();
    for( sal_Int32 i = 0; i < nAddProps; i++ )
        aCtrl[ nUsed++ ] = i_rControlOptions.maAddProps[i];

    SAL_WARN_IF( nUsed != nElements, "vcl.gdi", "nUsed != nElements, probable heap corruption" );

    return css::uno::makeAny( aCtrl );
}

css::uno::Any PrinterOptionsHelper::setGroupControlOpt(const OUString& i_rID,
                                             const OUString& i_rTitle,
                                             const OUString& i_rHelpId)
{
    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Group");
}

css::uno::Any PrinterOptionsHelper::setSubgroupControlOpt(const OUString& i_rID,
                                                const OUString& i_rTitle,
                                                const OUString& i_rHelpId,
                                                const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Subgroup", nullptr, i_rControlOptions);
}

css::uno::Any PrinterOptionsHelper::setBoolControlOpt(const OUString& i_rID,
                                            const OUString& i_rTitle,
                                            const OUString& i_rHelpId,
                                            const OUString& i_rProperty,
                                            bool i_bValue,
                                            const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = css::uno::makeAny( i_bValue );
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Bool", &aVal, i_rControlOptions);
}

css::uno::Any PrinterOptionsHelper::setChoiceRadiosControlOpt(const css::uno::Sequence< OUString >& i_rIDs,
                                              const OUString& i_rTitle,
                                              const css::uno::Sequence< OUString >& i_rHelpId,
                                              const OUString& i_rProperty,
                                              const css::uno::Sequence< OUString >& i_rChoices,
                                              sal_Int32 i_nValue,
                                              const css::uno::Sequence< sal_Bool >& i_rDisabledChoices,
                                              const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    sal_Int32 nUsed = aOpt.maAddProps.size();
    aOpt.maAddProps.resize( nUsed + 1 + (i_rDisabledChoices.getLength() ? 1 : 0) );
    aOpt.maAddProps[nUsed].Name = "Choices";
    aOpt.maAddProps[nUsed].Value = css::uno::makeAny( i_rChoices );
    if( i_rDisabledChoices.getLength() )
    {
        aOpt.maAddProps[nUsed+1].Name = "ChoicesDisabled";
        aOpt.maAddProps[nUsed+1].Value = css::uno::makeAny( i_rDisabledChoices );
    }

    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = css::uno::makeAny( i_nValue );
    return setUIControlOpt(i_rIDs, i_rTitle, i_rHelpId, "Radio", &aVal, aOpt);
}

css::uno::Any PrinterOptionsHelper::setChoiceListControlOpt(const OUString& i_rID,
                                              const OUString& i_rTitle,
                                              const css::uno::Sequence< OUString >& i_rHelpId,
                                              const OUString& i_rProperty,
                                              const css::uno::Sequence< OUString >& i_rChoices,
                                              sal_Int32 i_nValue,
                                              const css::uno::Sequence< sal_Bool >& i_rDisabledChoices,
                                              const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    sal_Int32 nUsed = aOpt.maAddProps.size();
    aOpt.maAddProps.resize( nUsed + 1 + (i_rDisabledChoices.getLength() ? 1 : 0) );
    aOpt.maAddProps[nUsed].Name = "Choices";
    aOpt.maAddProps[nUsed].Value = css::uno::makeAny( i_rChoices );
    if( i_rDisabledChoices.getLength() )
    {
        aOpt.maAddProps[nUsed+1].Name = "ChoicesDisabled";
        aOpt.maAddProps[nUsed+1].Value = css::uno::makeAny( i_rDisabledChoices );
    }

    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = css::uno::makeAny( i_nValue );
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, i_rHelpId, "List", &aVal, aOpt);
}

css::uno::Any PrinterOptionsHelper::setRangeControlOpt(const OUString& i_rID,
                                             const OUString& i_rTitle,
                                             const OUString& i_rHelpId,
                                             const OUString& i_rProperty,
                                             sal_Int32 i_nValue,
                                             sal_Int32 i_nMinValue,
                                             sal_Int32 i_nMaxValue,
                                             const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    if( i_nMaxValue >= i_nMinValue )
    {
        sal_Int32 nUsed = aOpt.maAddProps.size();
        aOpt.maAddProps.resize( nUsed + 2 );
        aOpt.maAddProps[nUsed  ].Name  = "MinValue";
        aOpt.maAddProps[nUsed++].Value = css::uno::makeAny( i_nMinValue );
        aOpt.maAddProps[nUsed  ].Name  = "MaxValue";
        aOpt.maAddProps[nUsed++].Value = css::uno::makeAny( i_nMaxValue );
    }

    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = css::uno::makeAny( i_nValue );
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Range", &aVal, aOpt);
}

css::uno::Any PrinterOptionsHelper::setEditControlOpt(const OUString& i_rID,
                                            const OUString& i_rTitle,
                                            const OUString& i_rHelpId,
                                            const OUString& i_rProperty,
                                            const OUString& i_rValue,
                                            const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    css::uno::Sequence< OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    css::beans::PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = css::uno::makeAny( i_rValue );
    css::uno::Sequence< OUString > aIds { i_rID };
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Edit", &aVal, i_rControlOptions);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
