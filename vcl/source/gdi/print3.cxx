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


#include "vcl/print.hxx"
#include "vcl/svapp.hxx"
#include "vcl/metaact.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/configsettings.hxx"
#include "vcl/unohelp.hxx"

#include "printdlg.hxx"
#include "svdata.hxx"
#include "salinst.hxx"
#include "salprn.hxx"
#include "svids.hrc"

#include "tools/urlobj.hxx"

#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/view/DuplexMode.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/awt/Size.hpp"
#include "comphelper/processfactory.hxx"

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace vcl;

class ImplPageCache
{
    struct CacheEntry
    {
        GDIMetaFile                 aPage;
        PrinterController::PageSize aSize;
    };

    std::vector< CacheEntry >  maPages;
    std::vector< sal_Int32 >    maPageNumbers;
    std::vector< sal_Int32 >    maCacheRanking;

    static const sal_Int32 nCacheSize = 6;

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
        rtl::OUString       maDependsOnName;
        sal_Int32           mnDependsOnEntry;

        ControlDependency() : mnDependsOnEntry( -1 ) {}
    };

    typedef boost::unordered_map< rtl::OUString, size_t, rtl::OUStringHash > PropertyToIndexMap;
    typedef boost::unordered_map< rtl::OUString, ControlDependency, rtl::OUStringHash > ControlDependencyMap;
    typedef boost::unordered_map< rtl::OUString, Sequence< sal_Bool >, rtl::OUStringHash > ChoiceDisableMap;

    boost::shared_ptr<Printer>                                  mpPrinter;
    Sequence< PropertyValue >                                   maUIOptions;
    std::vector< PropertyValue >                                maUIProperties;
    std::vector< bool >                                         maUIPropertyEnabled;
    PropertyToIndexMap                                          maPropertyToIndex;
    Link                                                        maOptionChangeHdl;
    ControlDependencyMap                                        maControlDependencies;
    ChoiceDisableMap                                            maChoiceDisableMap;
    sal_Bool                                                    mbFirstPage;
    sal_Bool                                                    mbLastPage;
    sal_Bool                                                    mbReversePageOrder;
    view::PrintableState                                        meJobState;

    vcl::PrinterController::MultiPageSetup                      maMultiPage;

    vcl::PrintProgressDialog*                                   mpProgress;

    ImplPageCache                                               maPageCache;

    // set by user through printer config dialog
    // if set, pages are centered and trimmed onto the fixed page
    Size                                                        maFixedPageSize;
    // set by user through printer config dialog
    sal_Int32                                                   mnDefaultPaperBin;
    // Set by user through printer preferences in print dialog.
    // Overrides application-set tray for a page.
    sal_Int32                                                   mnFixedPaperBin;

    // N.B. Apparently we have three levels of paper tray settings
    // (latter overrides former):
    // 1. default tray
    // 2. tray set for a concrete page by an application, e.g., writer
    //    allows setting a printer tray (for the default printer) for a
    //    page style. This setting can be overriden by user by selecting
    //    "Use only paper tray from printer preferences" on the Options
    //    page in the print dialog, in which case the default tray is
    //    used for all pages.
    // 3. tray set in printer properties the printer dialog
    // I'm not quite sure why 1. and 3. are distinct, but the commit
    // history suggests this is intentional...

    ImplPrinterControllerData() :
        mbFirstPage( sal_True ),
        mbLastPage( sal_False ),
        mbReversePageOrder( sal_False ),
        meJobState( view::PrintableState_JOB_STARTED ),
        mpProgress( NULL ),
        mnDefaultPaperBin( -1 ),
        mnFixedPaperBin( -1 )
    {}
    ~ImplPrinterControllerData() { delete mpProgress; }

    Size getRealPaperSize( const Size& i_rPageSize, bool bNoNUP ) const
    {
        if( maFixedPageSize.Width() > 0 && maFixedPageSize.Height() > 0 )
            return maFixedPageSize;
        if( maMultiPage.nRows * maMultiPage.nColumns > 1 && ! bNoNUP )
            return maMultiPage.aPaperSize;
        return i_rPageSize;
    }
    bool isFixedPageSize() const
    { return maFixedPageSize.Width() != 0 && maFixedPageSize.Height() != 0; }
    PrinterController::PageSize modifyJobSetup( const Sequence< PropertyValue >& i_rProps, bool bNoNUP );
};

PrinterController::PrinterController( const boost::shared_ptr<Printer>& i_pPrinter )
    : mpImplData( new ImplPrinterControllerData )
{
    mpImplData->mpPrinter = i_pPrinter;
}

static rtl::OUString queryFile( Printer* pPrinter )
{
    rtl::OUString aResult;

    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    if( xFactory.is() )
    {
        uno::Sequence< uno::Any > aTempl( 1 );
        aTempl.getArray()[0] <<= ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION;
        uno::Reference< ui::dialogs::XFilePicker > xFilePicker(
            xFactory->createInstanceWithArguments(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ) ),
                aTempl ), uno::UNO_QUERY );
        DBG_ASSERT( xFilePicker.is(), "could not get FilePicker service" );

        uno::Reference< ui::dialogs::XFilterManager > xFilterMgr( xFilePicker, uno::UNO_QUERY );
        if( xFilePicker.is() && xFilterMgr.is() )
        {
            try
            {
#ifdef UNX
                // add PostScript and PDF
                bool bPS = true, bPDF = true;
                if( pPrinter )
                {
                    if( pPrinter->GetCapabilities( PRINTER_CAPABILITIES_PDF ) )
                        bPS = false;
                    else
                        bPDF = false;
                }
                if( bPS )
                    xFilterMgr->appendFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PostScript" ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.ps" ) ) );
                if( bPDF )
                    xFilterMgr->appendFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Portable Document Format" ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.pdf" ) ) );
#elif defined WNT
        (void)pPrinter;
                xFilterMgr->appendFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.PRN" ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.prn" ) ) );
#endif
                // add arbitrary files
                xFilterMgr->appendFilter(VclResId(SV_STDTEXT_ALLFILETYPES), "*.*");
            }
            catch (const lang::IllegalArgumentException&)
            {
                SAL_WARN( "vcl.gdi", "caught IllegalArgumentException when registering filter" );
            }

            if( xFilePicker->execute() == ui::dialogs::ExecutableDialogResults::OK )
            {
                uno::Sequence< ::rtl::OUString > aPathSeq( xFilePicker->getFiles() );
                INetURLObject aObj( aPathSeq[0] );
                aResult = aObj.PathToFileName();
            }
        }
    }
    return aResult;
}

struct PrintJobAsync
{
    boost::shared_ptr<PrinterController>  mpController;
    JobSetup                            maInitSetup;

    PrintJobAsync( const boost::shared_ptr<PrinterController>& i_pController,
                   const JobSetup& i_rInitSetup
                   )
    : mpController( i_pController ), maInitSetup( i_rInitSetup )
    {}

    DECL_LINK( ExecJob, void* );
};

IMPL_LINK_NOARG(PrintJobAsync, ExecJob)
{
    Printer::ImplPrintJob( mpController, maInitSetup );

    // clean up, do not access members after this
    delete this;

    return 0;
}

void Printer::PrintJob( const boost::shared_ptr<PrinterController>& i_pController,
                        const JobSetup& i_rInitSetup
                        )
{
    sal_Bool bSynchronous = sal_False;
    beans::PropertyValue* pVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Wait" ) ) );
    if( pVal )
        pVal->Value >>= bSynchronous;

    if( bSynchronous )
        ImplPrintJob( i_pController, i_rInitSetup );
    else
    {
        PrintJobAsync* pAsync = new PrintJobAsync( i_pController, i_rInitSetup );
        Application::PostUserEvent( LINK( pAsync, PrintJobAsync, ExecJob ) );
    }
}

void Printer::ImplPrintJob( const boost::shared_ptr<PrinterController>& i_pController,
                            const JobSetup& i_rInitSetup
                            )
{
    boost::shared_ptr<PrinterController> pController( i_pController );

    // check if there is a default printer; if not, show an error box (if appropriate)
    if( GetDefaultPrinterName().isEmpty() )
    {
        if(  pController->isShowDialogs()
             // && ! pController->isDirectPrint()
           )
        {
            ErrorBox aBox( NULL, VclResId( SV_PRINT_NOPRINTERWARNING ) );
            aBox.Execute();
        }
        pController->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDirect" ) ),
                               makeAny( sal_False ) );
    }

    // setup printer

    // #i114306# changed behavior back from persistence
    // if no specific printer is already set, create the default printer
    if( ! pController->getPrinter() )
    {
        rtl::OUString aPrinterName( i_rInitSetup.GetPrinterName() );
        boost::shared_ptr<Printer> pPrinter( new Printer( aPrinterName ) );
        pPrinter->SetJobSetup( i_rInitSetup );
        pController->setPrinter( pPrinter );
    }

    // reset last page property
    i_pController->setLastPage( sal_False );

    // update "PageRange" property inferring from other properties:
    // case 1: "Pages" set from UNO API ->
    //         setup "Print Selection" and insert "PageRange" attribute
    // case 2: "All pages" is selected
    //         update "Page range" attribute to have a sensible default,
    //         but leave "All" as selected

    // "Pages" attribute from API is now equivalent to "PageRange"
    // AND "PrintContent" = 1 except calc where it is "PrintRange" = 1
    // Argh ! That sure needs cleaning up
    beans::PropertyValue* pContentVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) ) );
    if( ! pContentVal )
        pContentVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintContent" ) ) );

    // case 1: UNO API has set "Pages"
    beans::PropertyValue* pPagesVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Pages" ) ) );
    if( pPagesVal )
    {
        rtl::OUString aPagesVal;
        pPagesVal->Value >>= aPagesVal;
        if( !aPagesVal.isEmpty() )
        {
            // "Pages" attribute from API is now equivalent to "PageRange"
            // AND "PrintContent" = 1 except calc where it is "PrintRange" = 1
            // Argh ! That sure needs cleaning up
            if( pContentVal )
            {
                pContentVal->Value = makeAny( sal_Int32( 1 ) );
                i_pController->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ), pPagesVal->Value );
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
                beans::PropertyValue* pRangeVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ) );
                rtl::OUString aRange;
                if( pRangeVal )
                    pRangeVal->Value >>= aRange;
                if( aRange.isEmpty() )
                {
                    sal_Int32 nPages = i_pController->getPageCount();
                    if( nPages > 0 )
                    {
                        rtl::OUStringBuffer aBuf( 32 );
                        aBuf.appendAscii( "1" );
                        if( nPages > 1 )
                        {
                            aBuf.appendAscii( "-" );
                            aBuf.append( nPages );
                        }
                        i_pController->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ), makeAny( aBuf.makeStringAndClear() ) );
                    }
                }
            }
        }
    }

    beans::PropertyValue* pReverseVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintReverse" ) ) );
    if( pReverseVal )
    {
        sal_Bool bReverse = sal_False;
        pReverseVal->Value >>= bReverse;
        pController->setReversePrint( bReverse );
    }

    // in direct print case check whether there is anything to print.
    // if not, show an errorbox (if appropriate)
    if( pController->isShowDialogs() && pController->isDirectPrint() )
    {
        if( pController->getFilteredPageCount() == 0 )
        {
            ErrorBox aBox( NULL, VclResId( SV_PRINT_NOCONTENT ) );
            aBox.Execute();
            return;
        }
    }

    // check if the printer brings up its own dialog
    // in that case leave the work to that dialog
    if( ! pController->getPrinter()->GetCapabilities( PRINTER_CAPABILITIES_EXTERNALDIALOG ) &&
        ! pController->isDirectPrint() &&
        pController->isShowDialogs()
        )
    {
        try
        {
            PrintDialog aDlg( NULL, i_pController );
            if( ! aDlg.Execute() )
            {
                i_pController->abortJob();
                return;
            }
            if( aDlg.isPrintToFile() )
            {
                rtl::OUString aFile = queryFile( pController->getPrinter().get() );
                if( aFile.isEmpty() )
                {
                    i_pController->abortJob();
                    return;
                }
                pController->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LocalFileName" ) ),
                                       makeAny( aFile ) );
            }
            else if( aDlg.isSingleJobs() )
            {
                pController->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintCollateAsSingleJobs" ) ),
                                       makeAny( sal_True ) );
            }
        }
        catch (const std::bad_alloc&)
        {
        }
    }

    pController->pushPropertiesToPrinter();

    rtl::OUString aJobName;
    beans::PropertyValue* pJobNameVal = pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "JobName" ) ) );
    if( pJobNameVal )
        pJobNameVal->Value >>= aJobName;

    pController->getPrinter()->StartJob( String( aJobName ), pController );

    pController->jobFinished( pController->getJobState() );
}

bool Printer::StartJob( const rtl::OUString& i_rJobName, boost::shared_ptr<vcl::PrinterController>& i_pController )
{
    mnError = PRINTER_OK;

    if ( IsDisplayPrinter() )
        return sal_False;

    if ( IsJobActive() || IsPrinting() )
        return sal_False;

    sal_uLong   nCopies = mnCopyCount;
    bool    bCollateCopy = mbCollateCopy;
    bool    bUserCopy = sal_False;

    if ( nCopies > 1 )
    {
        sal_uLong nDevCopy;

        if ( bCollateCopy )
            nDevCopy = GetCapabilities( PRINTER_CAPABILITIES_COLLATECOPIES );
        else
            nDevCopy = GetCapabilities( PRINTER_CAPABILITIES_COPIES );

        // need to do copies by hand ?
        if ( nCopies > nDevCopy )
        {
            bUserCopy = sal_True;
            nCopies = 1;
            bCollateCopy = sal_False;
        }
    }
    else
        bCollateCopy = sal_False;


    ImplSVData* pSVData = ImplGetSVData();
    mpPrinter = pSVData->mpDefInst->CreatePrinter( mpInfoPrinter );

    if ( !mpPrinter )
        return sal_False;

    sal_Bool bSinglePrintJobs = sal_False;
    beans::PropertyValue* pSingleValue = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintCollateAsSingleJobs" ) ) );
    if( pSingleValue )
    {
        pSingleValue->Value >>= bSinglePrintJobs;
    }

    beans::PropertyValue* pFileValue = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LocalFileName" ) ) );
    if( pFileValue )
    {
        rtl::OUString aFile;
        pFileValue->Value >>= aFile;
        if( !aFile.isEmpty() )
        {
            mbPrintFile = sal_True;
            maPrintFile = aFile;
            bSinglePrintJobs = sal_False;
        }
    }

    rtl::OUString* pPrintFile = NULL;
    if ( mbPrintFile )
        pPrintFile = &maPrintFile;
    mpPrinterOptions->ReadFromConfig( mbPrintFile );

    maJobName               = i_rJobName;
    mnCurPage               = 1;
    mnCurPrintPage          = 1;
    mbPrinting              = sal_True;
    if( GetCapabilities( PRINTER_CAPABILITIES_USEPULLMODEL ) )
    {
        mbJobActive             = sal_True;
        // sallayer does all necessary page printing
        // and also handles showing a dialog
        // that also means it must call jobStarted when the dialog is finished
        // it also must set the JobState of the Controller
        if( mpPrinter->StartJob( pPrintFile,
                                 i_rJobName,
                                 Application::GetDisplayName(),
                                 maJobSetup.ImplGetConstData(),
                                 *i_pController ) )
        {
            EndJob();
        }
        else
        {
            mnError = ImplSalPrinterErrorCodeToVCL( mpPrinter->GetErrorCode() );
            if ( !mnError )
                mnError = PRINTER_GENERALERROR;
            pSVData->mpDefInst->DestroyPrinter( mpPrinter );
            mnCurPage           = 0;
            mnCurPrintPage      = 0;
            mbPrinting          = sal_False;
            mpPrinter = NULL;
            mbJobActive = sal_False;

            GDIMetaFile aDummyFile;
            i_pController->setLastPage(sal_True);
            i_pController->getFilteredPageFile(0, aDummyFile);

            return false;
        }
    }
    else
    {
        // possibly a dialog has been shown
        // now the real job starts
        i_pController->setJobState( view::PrintableState_JOB_STARTED );
        i_pController->jobStarted();

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
            bool bError = false, bAborted = false;
            if( mpPrinter->StartJob( pPrintFile,
                                     i_rJobName,
                                     Application::GetDisplayName(),
                                     nCopies,
                                     bCollateCopy,
                                     i_pController->isDirectPrint(),
                                     maJobSetup.ImplGetConstData() ) )
            {
                mbJobActive             = sal_True;
                i_pController->createProgressDialog();
                int nPages = i_pController->getFilteredPageCount();
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
                                i_pController->setLastPage( sal_True );
                            }
                            i_pController->printFilteredPage( nPage );
                            if( i_pController->isProgressCanceled() )
                            {
                                i_pController->abortJob();
                            }
                            if (i_pController->getJobState() ==
                                    view::PrintableState_JOB_ABORTED)
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
                        mbPrinting              = sal_True;
                    }
                    else
                        bError = true;
                }
            }
            else
                bError = true;

            if( bError )
            {
                mnError = ImplSalPrinterErrorCodeToVCL( mpPrinter->GetErrorCode() );
                if ( !mnError )
                    mnError = PRINTER_GENERALERROR;
                i_pController->setJobState( mnError == PRINTER_ABORT
                                            ? view::PrintableState_JOB_ABORTED
                                            : view::PrintableState_JOB_FAILED );
                if( mpPrinter )
                    pSVData->mpDefInst->DestroyPrinter( mpPrinter );
                mnCurPage           = 0;
                mnCurPrintPage      = 0;
                mbPrinting          = sal_False;
                mpPrinter = NULL;

                return false;
            }
        }

        if( i_pController->getJobState() == view::PrintableState_JOB_STARTED )
            i_pController->setJobState( view::PrintableState_JOB_SPOOLED );
    }

    // make last used printer persistent for UI jobs
    if( i_pController->isShowDialogs() && ! i_pController->isDirectPrint() )
    {
        SettingsConfigItem* pItem = SettingsConfigItem::get();
        pItem->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintDialog" ) ),
                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LastPrinterUsed" ) ),
                         GetName()
                         );
    }

    return true;
}

PrinterController::~PrinterController()
{
    delete mpImplData;
}

view::PrintableState PrinterController::getJobState() const
{
    return mpImplData->meJobState;
}

void PrinterController::setJobState( view::PrintableState i_eState )
{
    mpImplData->meJobState = i_eState;
}

const boost::shared_ptr<Printer>& PrinterController::getPrinter() const
{
    return mpImplData->mpPrinter;
}

void PrinterController::setPrinter( const boost::shared_ptr<Printer>& i_rPrinter )
{
    mpImplData->mpPrinter = i_rPrinter;
    setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ),
              makeAny( rtl::OUString( i_rPrinter->GetName() ) ) );
    mpImplData->mnDefaultPaperBin = mpImplData->mpPrinter->GetPaperBin();
    mpImplData->mnFixedPaperBin = -1;
}

void PrinterController:: resetPrinterOptions( bool i_bFileOutput )
{
    PrinterOptions aOpt;
    aOpt.ReadFromConfig( i_bFileOutput );
    mpImplData->mpPrinter->SetPrinterOptions( aOpt );
}

bool PrinterController::setupPrinter( Window* i_pParent )
{
    bool bRet = false;
    if( mpImplData->mpPrinter.get() )
    {
        // get old data
        Size aPaperSize( mpImplData->mpPrinter->PixelToLogic(
            mpImplData->mpPrinter->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );
        sal_uInt16 nPaperBin = mpImplData->mpPrinter->GetPaperBin();

        // call driver setup
        bRet = mpImplData->mpPrinter->Setup( i_pParent );
        if( bRet )
        {
            // was papersize or bin  overridden ? if so we need to take action
            Size aNewPaperSize( mpImplData->mpPrinter->PixelToLogic(
                mpImplData->mpPrinter->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );
            sal_uInt16 nNewPaperBin = mpImplData->mpPrinter->GetPaperBin();
            if( aNewPaperSize != aPaperSize || nNewPaperBin != nPaperBin )
            {
                mpImplData->maFixedPageSize = aNewPaperSize;
                mpImplData->maPageCache.invalidate();
                awt::Size aOverrideSize;
                aOverrideSize.Width = aNewPaperSize.Width();
                aOverrideSize.Height = aNewPaperSize.Height();
                setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OverridePageSize" ) ),
                          makeAny( aOverrideSize ) );
                mpImplData->mnFixedPaperBin = nNewPaperBin;
            }
        }
    }
    return bRet;
}

PrinterController::PageSize vcl::ImplPrinterControllerData::modifyJobSetup( const Sequence< PropertyValue >& i_rProps, bool bNoNUP )
{
    PrinterController::PageSize aPageSize;
    aPageSize.aSize = mpPrinter->GetPaperSize();
    awt::Size aSetSize, aIsSize;
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
            sal_Bool bVal = sal_False;
            i_rProps[ nProperty ].Value >>= bVal;
            aPageSize.bFullPaper = static_cast<bool>(bVal);
        }
        else if ( i_rProps[ nProperty ].Name == "PrinterPaperTray" )
        {
            sal_Int32 nBin = -1;
            i_rProps[ nProperty ].Value >>= nBin;
            if( nBin >= 0 && nBin < mpPrinter->GetPaperBinCount() )
                nPaperBin = nBin;
        }
    }

    Size aCurSize( mpPrinter->GetPaperSize() );
    if( aSetSize.Width && aSetSize.Height )
    {
        Size aSetPaperSize( aSetSize.Width, aSetSize.Height );
        Size aRealPaperSize( getRealPaperSize( aSetPaperSize, bNoNUP ) );
        if( aRealPaperSize != aCurSize )
            aIsSize = aSetSize;
    }

    if( aIsSize.Width && aIsSize.Height )
    {
        aPageSize.aSize.Width() = aIsSize.Width;
        aPageSize.aSize.Height() = aIsSize.Height;

        Size aRealPaperSize( getRealPaperSize( aPageSize.aSize, bNoNUP ) );
        if( aRealPaperSize != aCurSize )
            mpPrinter->SetPaperSizeUser( aRealPaperSize, ! isFixedPageSize() );
    }

    // paper bin set from properties in print dialog overrides
    // application default for a page
    if ( mnFixedPaperBin != -1 )
        nPaperBin = mnFixedPaperBin;

    if( nPaperBin != -1 && nPaperBin != mpPrinter->GetPaperBin() )
        mpPrinter->SetPaperBin( nPaperBin );

    return aPageSize;
}

int PrinterController::getPageCountProtected() const
{
    const MapMode aMapMode( MAP_100TH_MM );

    mpImplData->mpPrinter->Push();
    mpImplData->mpPrinter->SetMapMode( aMapMode );
    int nPages = getPageCount();
    mpImplData->mpPrinter->Pop();
    return nPages;
}

Sequence< beans::PropertyValue > PrinterController::getPageParametersProtected( int i_nPage ) const
{
    const MapMode aMapMode( MAP_100TH_MM );

    mpImplData->mpPrinter->Push();
    mpImplData->mpPrinter->SetMapMode( aMapMode );
    Sequence< beans::PropertyValue > aResult( getPageParameters( i_nPage ) );
    mpImplData->mpPrinter->Pop();
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
    Sequence< PropertyValue > aPageParm( getPageParametersProtected( i_nUnfilteredPage ) );
    const MapMode aMapMode( MAP_100TH_MM );

    mpImplData->mpPrinter->Push();
    mpImplData->mpPrinter->SetMapMode( aMapMode );

    // modify job setup if necessary
    PrinterController::PageSize aPageSize = mpImplData->modifyJobSetup( aPageParm, true );

    o_rMtf.SetPrefSize( aPageSize.aSize );
    o_rMtf.SetPrefMapMode( aMapMode );

    mpImplData->mpPrinter->EnableOutput( sal_False );

    o_rMtf.Record( mpImplData->mpPrinter.get() );

    printPage( i_nUnfilteredPage );

    o_rMtf.Stop();
    o_rMtf.WindStart();
    mpImplData->mpPrinter->Pop();

    if( i_bMayUseCache )
        mpImplData->maPageCache.insert( i_nUnfilteredPage, o_rMtf, aPageSize );

    // reset "FirstPage" property to false now we've gotten at least our first one
    mpImplData->mbFirstPage = sal_False;

    return aPageSize;
}

static void appendSubPage( GDIMetaFile& o_rMtf, const Rectangle& i_rClipRect, GDIMetaFile& io_rSubPage, bool i_bDrawBorder )
{
    // intersect all clipregion actions with our clip rect
    io_rSubPage.WindStart();
    io_rSubPage.Clip( i_rClipRect );

    // save gstate
    o_rMtf.AddAction( new MetaPushAction( PUSH_ALL ) );

    // clip to page rect
    o_rMtf.AddAction( new MetaClipRegionAction( Region( i_rClipRect ), sal_True ) );

    // append the subpage
    io_rSubPage.WindStart();
    io_rSubPage.Play( o_rMtf );

    // restore gstate
    o_rMtf.AddAction( new MetaPopAction() );

    // draw a border
    if( i_bDrawBorder )
    {
        // save gstate
        o_rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR | PUSH_FILLCOLOR | PUSH_CLIPREGION | PUSH_MAPMODE ) );
        o_rMtf.AddAction( new MetaMapModeAction( MapMode( MAP_100TH_MM ) ) );

        Rectangle aBorderRect( i_rClipRect );
        o_rMtf.AddAction( new MetaLineColorAction( Color( COL_BLACK ), sal_True ) );
        o_rMtf.AddAction( new MetaFillColorAction( Color( COL_TRANSPARENT ), sal_False ) );
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
        if (mpImplData->meJobState != view::PrintableState_JOB_STARTED)
        {   // rhbz#657394: check that we are still printing...
            return PrinterController::PageSize();
        }
        Size aPaperSize = mpImplData->getRealPaperSize( aPageSize.aSize, true );
        mpImplData->mpPrinter->SetMapMode( MapMode( MAP_100TH_MM ) );
        mpImplData->mpPrinter->SetPaperSizeUser( aPaperSize, ! mpImplData->isFixedPageSize() );
        if( aPaperSize != aPageSize.aSize )
        {
            // user overridden page size, center Metafile
            o_rMtf.WindStart();
            long nDX = (aPaperSize.Width() - aPageSize.aSize.Width()) / 2;
            long nDY = (aPaperSize.Height() - aPageSize.aSize.Height()) / 2;
            o_rMtf.Move( nDX, nDY, mpImplData->mpPrinter->ImplGetDPIX(), mpImplData->mpPrinter->ImplGetDPIY() );
            o_rMtf.WindStart();
            o_rMtf.SetPrefSize( aPaperSize );
            aPageSize.aSize = aPaperSize;
        }
        return aPageSize;
    }

    // set last page property really only on the very last page to be rendered
    // that is on the last subpage of a NUp run
    sal_Bool bIsLastPage = mpImplData->mbLastPage;
    mpImplData->mbLastPage = sal_False;

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
    o_rMtf.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
    o_rMtf.AddAction( new MetaMapModeAction( MapMode( MAP_100TH_MM ) ) );

    int nDocPages = getPageCountProtected();
    if (mpImplData->meJobState != view::PrintableState_JOB_STARTED)
    {   // rhbz#657394: check that we are still printing...
        return PrinterController::PageSize();
    }
    for( int nSubPage = 0; nSubPage < nSubPages; nSubPage++ )
    {
        // map current sub page to real page
        int nPage = (i_nFilteredPage * nSubPages + nSubPage) / rMPS.nRepeat;
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
                case PrinterController::LRTB:
                    nCellX = (nSubPage % rMPS.nColumns);
                    nCellY = (nSubPage / rMPS.nColumns);
                    break;
                case PrinterController::TBLR:
                    nCellX = (nSubPage / rMPS.nRows);
                    nCellY = (nSubPage % rMPS.nRows);
                    break;
                case PrinterController::RLTB:
                    nCellX = rMPS.nColumns - 1 - (nSubPage % rMPS.nColumns);
                    nCellY = (nSubPage / rMPS.nColumns);
                    break;
                case PrinterController::TBRL:
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
                aPageFile.Move( nX, nY, mpImplData->mpPrinter->ImplGetDPIX(), mpImplData->mpPrinter->ImplGetDPIY() );
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
    mpImplData->mpPrinter->SetMapMode( MapMode( MAP_100TH_MM ) );
    mpImplData->mpPrinter->SetPaperSizeUser( aPaperSize, ! mpImplData->isFixedPageSize() );

    return PrinterController::PageSize( aPaperSize, true );
}

int PrinterController::getFilteredPageCount()
{
    int nDiv = mpImplData->maMultiPage.nRows * mpImplData->maMultiPage.nColumns;
    if( nDiv < 1 )
        nDiv = 1;
    return (getPageCountProtected() * mpImplData->maMultiPage.nRepeat + (nDiv-1)) / nDiv;
}

sal_uLong PrinterController::removeTransparencies( GDIMetaFile& i_rIn, GDIMetaFile& o_rOut )
{
    sal_uLong nRestoreDrawMode = mpImplData->mpPrinter->GetDrawMode();
    sal_Int32 nMaxBmpDPIX = mpImplData->mpPrinter->ImplGetDPIX();
    sal_Int32 nMaxBmpDPIY = mpImplData->mpPrinter->ImplGetDPIY();

    const PrinterOptions&   rPrinterOptions = mpImplData->mpPrinter->GetPrinterOptions();

    static const sal_Int32 OPTIMAL_BMP_RESOLUTION = 300;
    static const sal_Int32 NORMAL_BMP_RESOLUTION  = 200;


    if( rPrinterOptions.IsReduceBitmaps() )
    {
        // calculate maximum resolution for bitmap graphics
        if( PRINTER_BITMAP_OPTIMAL == rPrinterOptions.GetReducedBitmapMode() )
        {
            nMaxBmpDPIX = std::min( sal_Int32(OPTIMAL_BMP_RESOLUTION), nMaxBmpDPIX );
            nMaxBmpDPIY = std::min( sal_Int32(OPTIMAL_BMP_RESOLUTION), nMaxBmpDPIY );
        }
        else if( PRINTER_BITMAP_NORMAL == rPrinterOptions.GetReducedBitmapMode() )
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

    // convert to greysacles
    if( rPrinterOptions.IsConvertToGreyscales() )
    {
        mpImplData->mpPrinter->SetDrawMode( mpImplData->mpPrinter->GetDrawMode() |
                                            ( DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_GRAYTEXT |
                                              DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT ) );
    }

    // disable transparency output
    if( rPrinterOptions.IsReduceTransparency() && ( PRINTER_TRANSPARENCY_NONE == rPrinterOptions.GetReducedTransparencyMode() ) )
    {
        mpImplData->mpPrinter->SetDrawMode( mpImplData->mpPrinter->GetDrawMode() | DRAWMODE_NOTRANSPARENCY );
    }

    Color aBg( COL_TRANSPARENT ); // default: let RemoveTransparenciesFromMetaFile do its own background logic
    if( mpImplData->maMultiPage.nRows * mpImplData->maMultiPage.nColumns > 1 )
    {
        // in N-Up printing we have no "page" background operation
        // we also have no way to determine the paper color
        // so let's go for white, which will kill 99.9% of the real cases
        aBg = Color( COL_WHITE );
    }
    mpImplData->mpPrinter->RemoveTransparenciesFromMetaFile( i_rIn, o_rOut, nMaxBmpDPIX, nMaxBmpDPIY,
                                                             rPrinterOptions.IsReduceTransparency(),
                                                             rPrinterOptions.GetReducedTransparencyMode() == PRINTER_TRANSPARENCY_AUTO,
                                                             rPrinterOptions.IsReduceBitmaps() && rPrinterOptions.IsReducedBitmapIncludesTransparency(),
                                                             aBg
                                                             );
    return nRestoreDrawMode;
}

void PrinterController::printFilteredPage( int i_nPage )
{
    if( mpImplData->meJobState != view::PrintableState_JOB_STARTED )
        return;

    GDIMetaFile aPageFile;
    PrinterController::PageSize aPageSize = getFilteredPageFile( i_nPage, aPageFile );

    if (mpImplData->meJobState != view::PrintableState_JOB_STARTED)
    {   // rhbz#657394: check that we are still printing...
        return;
    }

    if( mpImplData->mpProgress )
    {
        // do nothing if printing is canceled
        if( mpImplData->mpProgress->isCanceled() )
        {
            setJobState( view::PrintableState_JOB_ABORTED );
            return;
        }
    }

    // in N-Up printing set the correct page size
    mpImplData->mpPrinter->SetMapMode( MAP_100TH_MM );
    // aPageSize was filtered through mpImplData->getRealPaperSize already by getFilteredPageFile()
    mpImplData->mpPrinter->SetPaperSizeUser( aPageSize.aSize, ! mpImplData->isFixedPageSize() );
    if( mpImplData->mnFixedPaperBin != -1 &&
        mpImplData->mpPrinter->GetPaperBin() != mpImplData->mnFixedPaperBin )
    {
        mpImplData->mpPrinter->SetPaperBin( mpImplData->mnFixedPaperBin );
    }

    // if full paper is meant to be used, move the output to accomodate for pageoffset
    if( aPageSize.bFullPaper )
    {
        Point aPageOffset( mpImplData->mpPrinter->GetPageOffset() );
        aPageFile.WindStart();
        aPageFile.Move( -aPageOffset.X(), -aPageOffset.Y(), mpImplData->mpPrinter->ImplGetDPIX(), mpImplData->mpPrinter->ImplGetDPIY() );
    }

    GDIMetaFile aCleanedFile;
    sal_uLong nRestoreDrawMode = removeTransparencies( aPageFile, aCleanedFile );

    mpImplData->mpPrinter->EnableOutput( sal_True );

    // actually print the page
    mpImplData->mpPrinter->ImplStartPage();

    mpImplData->mpPrinter->Push();
    aCleanedFile.WindStart();
    aCleanedFile.Play( mpImplData->mpPrinter.get() );
    mpImplData->mpPrinter->Pop();

    mpImplData->mpPrinter->ImplEndPage();

    mpImplData->mpPrinter->SetDrawMode( nRestoreDrawMode );
}

void PrinterController::jobStarted()
{
}

void PrinterController::jobFinished( view::PrintableState )
{
}

void PrinterController::abortJob()
{
    setJobState( view::PrintableState_JOB_ABORTED );
    // applications (well, sw) depend on a page request with "IsLastPage" = true
    // to free resources, else they (well, sw) will crash eventually
    setLastPage( sal_True );
    delete mpImplData->mpProgress;
    mpImplData->mpProgress = NULL;
    GDIMetaFile aMtf;
    getPageFile( 0, aMtf, false );
}

void PrinterController::setLastPage( sal_Bool i_bLastPage )
{
    mpImplData->mbLastPage = i_bLastPage;
}

void PrinterController::setReversePrint( sal_Bool i_bReverse )
{
    mpImplData->mbReversePageOrder = i_bReverse;
}

bool PrinterController::getReversePrint() const
{
    return mpImplData->mbReversePageOrder;
}

Sequence< PropertyValue > PrinterController::getJobProperties( const Sequence< PropertyValue >& i_rMergeList ) const
{
    boost::unordered_set< rtl::OUString, rtl::OUStringHash > aMergeSet;
    size_t nResultLen = size_t(i_rMergeList.getLength()) + mpImplData->maUIProperties.size() + 3;
    for( int i = 0; i < i_rMergeList.getLength(); i++ )
        aMergeSet.insert( i_rMergeList[i].Name );

    Sequence< PropertyValue > aResult( nResultLen );
    for( int i = 0; i < i_rMergeList.getLength(); i++ )
        aResult[i] = i_rMergeList[i];
    int nCur = i_rMergeList.getLength();
    for( size_t i = 0; i < mpImplData->maUIProperties.size(); i++ )
    {
        if( aMergeSet.find( mpImplData->maUIProperties[i].Name ) == aMergeSet.end() )
            aResult[nCur++] = mpImplData->maUIProperties[i];
    }
    // append IsFirstPage
    if( aMergeSet.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFirstPage" ) ) ) == aMergeSet.end() )
    {
        PropertyValue aVal;
        aVal.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFirstPage" ) );
        aVal.Value <<= mpImplData->mbFirstPage;
        aResult[nCur++] = aVal;
    }
    // append IsLastPage
    if( aMergeSet.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsLastPage" ) ) ) == aMergeSet.end() )
    {
        PropertyValue aVal;
        aVal.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsLastPage" ) );
        aVal.Value <<= mpImplData->mbLastPage;
        aResult[nCur++] = aVal;
    }
    // append IsPrinter
    if( aMergeSet.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsPrinter" ) ) ) == aMergeSet.end() )
    {
        PropertyValue aVal;
        aVal.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsPrinter" ) );
        aVal.Value <<= sal_True;
        aResult[nCur++] = aVal;
    }
    aResult.realloc( nCur );
    return aResult;
}

const Sequence< beans::PropertyValue >& PrinterController::getUIOptions() const
{
    return mpImplData->maUIOptions;
}

beans::PropertyValue* PrinterController::getValue( const rtl::OUString& i_rProperty )
{
    boost::unordered_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    return it != mpImplData->maPropertyToIndex.end() ? &mpImplData->maUIProperties[it->second] : NULL;
}

const beans::PropertyValue* PrinterController::getValue( const rtl::OUString& i_rProperty ) const
{
    boost::unordered_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    return it != mpImplData->maPropertyToIndex.end() ? &mpImplData->maUIProperties[it->second] : NULL;
}

void PrinterController::setValue( const rtl::OUString& i_rName, const Any& i_rValue )
{
    beans::PropertyValue aVal;
    aVal.Name = i_rName;
    aVal.Value = i_rValue;

    setValue( aVal );
}

void PrinterController::setValue( const beans::PropertyValue& i_rValue )
{
    boost::unordered_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rValue.Name );
    if( it != mpImplData->maPropertyToIndex.end() )
        mpImplData->maUIProperties[ it->second ] = i_rValue;
    else
    {
        // insert correct index into property map
        mpImplData->maPropertyToIndex[ i_rValue.Name ] = mpImplData->maUIProperties.size();
        mpImplData->maUIProperties.push_back( i_rValue );
        mpImplData->maUIPropertyEnabled.push_back( true );
    }
}

void PrinterController::setUIOptions( const Sequence< beans::PropertyValue >& i_rOptions )
{
    DBG_ASSERT( mpImplData->maUIOptions.getLength() == 0, "setUIOptions called twice !" );

    mpImplData->maUIOptions = i_rOptions;

    for( int i = 0; i < i_rOptions.getLength(); i++ )
    {
        Sequence< beans::PropertyValue > aOptProp;
        i_rOptions[i].Value >>= aOptProp;
        bool bIsEnabled = true;
        bool bHaveProperty = false;
        rtl::OUString aPropName;
        vcl::ImplPrinterControllerData::ControlDependency aDep;
        Sequence< sal_Bool > aChoicesDisabled;
        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if ( rEntry.Name == "Property" )
            {
                PropertyValue aVal;
                rEntry.Value >>= aVal;
                DBG_ASSERT( mpImplData->maPropertyToIndex.find( aVal.Name )
                            == mpImplData->maPropertyToIndex.end(), "duplicate property entry" );
                setValue( aVal );
                aPropName = aVal.Name;
                bHaveProperty = true;
            }
            else if ( rEntry.Name == "Enabled" )
            {
                sal_Bool bValue = sal_True;
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

bool PrinterController::isUIOptionEnabled( const rtl::OUString& i_rProperty ) const
{
    bool bEnabled = false;
    boost::unordered_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator prop_it =
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
                    const com::sun::star::beans::PropertyValue* pVal = getValue( it->second.maDependsOnName );
                    OSL_ENSURE( pVal, "unknown property in dependency" );
                    if( pVal )
                    {
                        sal_Int32 nDepVal = 0;
                        sal_Bool bDepVal = sal_False;
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

bool PrinterController::isUIChoiceEnabled( const rtl::OUString& i_rProperty, sal_Int32 i_nValue ) const
{
    bool bEnabled = true;
    ImplPrinterControllerData::ChoiceDisableMap::const_iterator it =
        mpImplData->maChoiceDisableMap.find( i_rProperty );
    if(it != mpImplData->maChoiceDisableMap.end() )
    {
        const Sequence< sal_Bool >& rDisabled( it->second );
        if( i_nValue >= 0 && i_nValue < rDisabled.getLength() )
            bEnabled = ! rDisabled[i_nValue];
    }
    return bEnabled;
}

rtl::OUString PrinterController::getDependency( const rtl::OUString& i_rProperty ) const
{
    rtl::OUString aDependency;

    vcl::ImplPrinterControllerData::ControlDependencyMap::const_iterator it =
        mpImplData->maControlDependencies.find( i_rProperty );
    if( it != mpImplData->maControlDependencies.end() )
        aDependency = it->second.maDependsOnName;

    return aDependency;
}

rtl::OUString PrinterController::makeEnabled( const rtl::OUString& i_rProperty )
{
    rtl::OUString aDependency;

    vcl::ImplPrinterControllerData::ControlDependencyMap::const_iterator it =
        mpImplData->maControlDependencies.find( i_rProperty );
    if( it != mpImplData->maControlDependencies.end() )
    {
        if( isUIOptionEnabled( it->second.maDependsOnName ) )
        {
           aDependency = it->second.maDependsOnName;
           const com::sun::star::beans::PropertyValue* pVal = getValue( aDependency );
           OSL_ENSURE( pVal, "unknown property in dependency" );
           if( pVal )
           {
               sal_Int32 nDepVal = 0;
               sal_Bool bDepVal = sal_False;
               if( pVal->Value >>= nDepVal )
               {
                   if( it->second.mnDependsOnEntry != -1 )
                   {
                       setValue( aDependency, makeAny( sal_Int32( it->second.mnDependsOnEntry ) ) );
                   }
               }
               else if( pVal->Value >>= bDepVal )
               {
                   setValue( aDependency, makeAny( sal_Bool( it->second.mnDependsOnEntry != 0 ) ) );
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

void PrinterController::setOptionChangeHdl( const Link& i_rHdl )
{
    mpImplData->maOptionChangeHdl = i_rHdl;
}

void PrinterController::createProgressDialog()
{
    if( ! mpImplData->mpProgress )
    {
        sal_Bool bShow = sal_True;
        beans::PropertyValue* pMonitor = getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MonitorVisible" ) ) );
        if( pMonitor )
            pMonitor->Value >>= bShow;
        else
        {
            const com::sun::star::beans::PropertyValue* pVal = getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsApi" ) ) );
            if( pVal )
            {
                sal_Bool bApi = sal_False;
                pVal->Value >>= bApi;
                bShow = ! bApi;
            }
        }

        if( bShow && ! Application::IsHeadlessModeEnabled() )
        {
            mpImplData->mpProgress = new PrintProgressDialog( NULL, getPageCountProtected() );
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

void PrinterController::pushPropertiesToPrinter()
{
    sal_Int32 nCopyCount = 1;
    // set copycount and collate
    const beans::PropertyValue* pVal = getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CopyCount" ) ) );
    if( pVal )
        pVal->Value >>= nCopyCount;
    sal_Bool bCollate = sal_False;
    pVal = getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Collate" ) ) );
    if( pVal )
        pVal->Value >>= bCollate;
    mpImplData->mpPrinter->SetCopyCount( static_cast<sal_uInt16>(nCopyCount), bCollate );

    // duplex mode
    pVal = getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DuplexMode" ) ) );
    if( pVal )
    {
        sal_Int16 nDuplex = view::DuplexMode::UNKNOWN;
        pVal->Value >>= nDuplex;
        switch( nDuplex )
        {
        case view::DuplexMode::OFF: mpImplData->mpPrinter->SetDuplexMode( DUPLEX_OFF ); break;
        case view::DuplexMode::LONGEDGE: mpImplData->mpPrinter->SetDuplexMode( DUPLEX_LONGEDGE ); break;
        case view::DuplexMode::SHORTEDGE: mpImplData->mpPrinter->SetDuplexMode( DUPLEX_SHORTEDGE ); break;
        }
    }
}

bool PrinterController::isShowDialogs() const
{
    sal_Bool bApi = getBoolProperty( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsApi" ) ), sal_False );
    return ! bApi && ! Application::IsHeadlessModeEnabled();
}

bool PrinterController::isDirectPrint() const
{
    sal_Bool bDirect = getBoolProperty( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDirect" ) ), sal_False );
    return bDirect == sal_True;
}

sal_Bool PrinterController::getBoolProperty( const rtl::OUString& i_rProperty, sal_Bool i_bFallback ) const
{
    sal_Bool bRet = i_bFallback;
    const com::sun::star::beans::PropertyValue* pVal = getValue( i_rProperty );
    if( pVal )
        pVal->Value >>= bRet;
    return bRet;
}

/*
 * PrinterOptionsHelper
**/
Any PrinterOptionsHelper::getValue( const rtl::OUString& i_rPropertyName ) const
{
    Any aRet;
    boost::unordered_map< rtl::OUString, Any, rtl::OUStringHash >::const_iterator it =
        m_aPropertyMap.find( i_rPropertyName );
    if( it != m_aPropertyMap.end() )
        aRet = it->second;
    return aRet;
}

sal_Bool PrinterOptionsHelper::getBoolValue( const rtl::OUString& i_rPropertyName, sal_Bool i_bDefault ) const
{
    sal_Bool bRet = sal_False;
    Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= bRet) ? bRet : i_bDefault;
}

sal_Int64 PrinterOptionsHelper::getIntValue( const rtl::OUString& i_rPropertyName, sal_Int64 i_nDefault ) const
{
    sal_Int64 nRet = 0;
    Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= nRet) ? nRet : i_nDefault;
}

rtl::OUString PrinterOptionsHelper::getStringValue( const rtl::OUString& i_rPropertyName, const rtl::OUString& i_rDefault ) const
{
    rtl::OUString aRet;
    Any aVal( getValue( i_rPropertyName ) );
    return (aVal >>= aRet) ? aRet : i_rDefault;
}

bool PrinterOptionsHelper::processProperties( const Sequence< PropertyValue >& i_rNewProp,
                                              std::set< rtl::OUString >* o_pChangeProp )
{
    bool bChanged = false;

    // clear the changed set
    if( o_pChangeProp )
        o_pChangeProp->clear();

    sal_Int32 nElements = i_rNewProp.getLength();
    const PropertyValue* pVals = i_rNewProp.getConstArray();
    for( sal_Int32 i = 0; i < nElements; i++ )
    {
        bool bElementChanged = false;
        boost::unordered_map< rtl::OUString, Any, rtl::OUStringHash >::iterator it =
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
            if( o_pChangeProp )
                o_pChangeProp->insert( pVals[ i ].Name );
            m_aPropertyMap[ pVals[i].Name ] = pVals[i].Value;
            bChanged = true;
        }
    }
    return bChanged;
}

void PrinterOptionsHelper::appendPrintUIOptions( uno::Sequence< beans::PropertyValue >& io_rProps ) const
{
    if( m_aUIProperties.getLength() > 0 )
    {
        sal_Int32 nIndex = io_rProps.getLength();
        io_rProps.realloc( nIndex+1 );
        PropertyValue aVal;
        aVal.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ExtraPrintUIOptions" ) );
        aVal.Value = makeAny( m_aUIProperties );
        io_rProps[ nIndex ] = aVal;
    }
}

Any PrinterOptionsHelper::setUIControlOpt(const com::sun::star::uno::Sequence< rtl::OUString >& i_rIDs,
                                          const rtl::OUString& i_rTitle,
                                          const Sequence< rtl::OUString >& i_rHelpIds,
                                          const rtl::OUString& i_rType,
                                          const PropertyValue* i_pVal,
                                          const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    sal_Int32 nElements =
        2                                                             // ControlType + ID
        + (i_rTitle.isEmpty() ? 0 : 1)                                // Text
        + (i_rHelpIds.getLength() ? 1 : 0)                            // HelpId
        + (i_pVal ? 1 : 0)                                            // Property
        + i_rControlOptions.maAddProps.getLength()                    // additional props
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

    Sequence< PropertyValue > aCtrl( nElements );
    sal_Int32 nUsed = 0;
    if( !i_rTitle.isEmpty() )
    {
        aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ) );
        aCtrl[nUsed++].Value = makeAny( i_rTitle );
    }
    if( i_rHelpIds.getLength() )
    {
        aCtrl[nUsed  ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HelpId" ) );
        aCtrl[nUsed++].Value = makeAny( i_rHelpIds );
    }
    aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ControlType" ) );
    aCtrl[nUsed++].Value = makeAny( i_rType );
    aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ID" ) );
    aCtrl[nUsed++].Value = makeAny( i_rIDs );
    if( i_pVal )
    {
        aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Property" ) );
        aCtrl[nUsed++].Value = makeAny( *i_pVal );
    }
    if( !i_rControlOptions.maDependsOnName.isEmpty() )
    {
        aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DependsOnName" ) );
        aCtrl[nUsed++].Value = makeAny( i_rControlOptions.maDependsOnName );
        if( i_rControlOptions.mnDependsOnEntry != -1 )
        {
            aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DependsOnEntry" ) );
            aCtrl[nUsed++].Value = makeAny( i_rControlOptions.mnDependsOnEntry );
        }
        if( i_rControlOptions.mbAttachToDependency )
        {
            aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AttachToDependency" ) );
            aCtrl[nUsed++].Value = makeAny( i_rControlOptions.mbAttachToDependency );
        }
    }
    if( !i_rControlOptions.maGroupHint.isEmpty() )
    {
        aCtrl[nUsed  ].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GroupingHint" ) );
        aCtrl[nUsed++].Value <<= i_rControlOptions.maGroupHint;
    }
    if( i_rControlOptions.mbInternalOnly )
    {
        aCtrl[nUsed  ].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InternalUIOnly" ) );
        aCtrl[nUsed++].Value <<= sal_True;
    }
    if( ! i_rControlOptions.mbEnabled )
    {
        aCtrl[nUsed  ].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enabled" ) );
        aCtrl[nUsed++].Value <<= sal_False;
    }

    sal_Int32 nAddProps = i_rControlOptions.maAddProps.getLength();
    for( sal_Int32 i = 0; i < nAddProps; i++ )
        aCtrl[ nUsed++ ] = i_rControlOptions.maAddProps[i];

    DBG_ASSERT( nUsed == nElements, "nUsed != nElements, probable heap corruption" );

    return makeAny( aCtrl );
}

Any PrinterOptionsHelper::setGroupControlOpt(const rtl::OUString& i_rID,
                                             const rtl::OUString& i_rTitle,
                                             const rtl::OUString& i_rHelpId)
{
    Sequence< rtl::OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    Sequence< rtl::OUString > aIds(1);
    aIds[0] = i_rID;
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Group");
}

Any PrinterOptionsHelper::setSubgroupControlOpt(const rtl::OUString& i_rID,
                                                const rtl::OUString& i_rTitle,
                                                const rtl::OUString& i_rHelpId,
                                                const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    Sequence< rtl::OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    Sequence< rtl::OUString > aIds(1);
    aIds[0] = i_rID;
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Subgroup", NULL, i_rControlOptions);
}

Any PrinterOptionsHelper::setBoolControlOpt(const rtl::OUString& i_rID,
                                            const rtl::OUString& i_rTitle,
                                            const rtl::OUString& i_rHelpId,
                                            const rtl::OUString& i_rProperty,
                                            sal_Bool i_bValue,
                                            const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    Sequence< rtl::OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_bValue );
    Sequence< rtl::OUString > aIds(1);
    aIds[0] = i_rID;
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Bool", &aVal, i_rControlOptions);
}

Any PrinterOptionsHelper::setChoiceRadiosControlOpt(const com::sun::star::uno::Sequence< rtl::OUString >& i_rIDs,
                                              const rtl::OUString& i_rTitle,
                                              const Sequence< rtl::OUString >& i_rHelpId,
                                              const rtl::OUString& i_rProperty,
                                              const Sequence< rtl::OUString >& i_rChoices,
                                              sal_Int32 i_nValue,
                                              const Sequence< sal_Bool >& i_rDisabledChoices,
                                              const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    sal_Int32 nUsed = aOpt.maAddProps.getLength();
    aOpt.maAddProps.realloc( nUsed + 1 + (i_rDisabledChoices.getLength() ? 1 : 0) );
    aOpt.maAddProps[nUsed].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Choices" ) );
    aOpt.maAddProps[nUsed].Value = makeAny( i_rChoices );
    if( i_rDisabledChoices.getLength() )
    {
        aOpt.maAddProps[nUsed+1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ChoicesDisabled" ) );
        aOpt.maAddProps[nUsed+1].Value = makeAny( i_rDisabledChoices );
    }

    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_nValue );
    return setUIControlOpt(i_rIDs, i_rTitle, i_rHelpId, "Radio", &aVal, aOpt);
}

Any PrinterOptionsHelper::setChoiceListControlOpt(const rtl::OUString& i_rID,
                                              const rtl::OUString& i_rTitle,
                                              const Sequence< rtl::OUString >& i_rHelpId,
                                              const rtl::OUString& i_rProperty,
                                              const Sequence< rtl::OUString >& i_rChoices,
                                              sal_Int32 i_nValue,
                                              const Sequence< sal_Bool >& i_rDisabledChoices,
                                              const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    sal_Int32 nUsed = aOpt.maAddProps.getLength();
    aOpt.maAddProps.realloc( nUsed + 1 + (i_rDisabledChoices.getLength() ? 1 : 0) );
    aOpt.maAddProps[nUsed].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Choices" ) );
    aOpt.maAddProps[nUsed].Value = makeAny( i_rChoices );
    if( i_rDisabledChoices.getLength() )
    {
        aOpt.maAddProps[nUsed+1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ChoicesDisabled" ) );
        aOpt.maAddProps[nUsed+1].Value = makeAny( i_rDisabledChoices );
    }

    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_nValue );
    Sequence< rtl::OUString > aIds(1);
    aIds[0] = i_rID;
    return setUIControlOpt(aIds, i_rTitle, i_rHelpId, "List", &aVal, aOpt);
}

Any PrinterOptionsHelper::setRangeControlOpt(const rtl::OUString& i_rID,
                                             const rtl::OUString& i_rTitle,
                                             const rtl::OUString& i_rHelpId,
                                             const rtl::OUString& i_rProperty,
                                             sal_Int32 i_nValue,
                                             sal_Int32 i_nMinValue,
                                             sal_Int32 i_nMaxValue,
                                             const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    UIControlOptions aOpt( i_rControlOptions );
    if( i_nMaxValue >= i_nMinValue )
    {
        sal_Int32 nUsed = aOpt.maAddProps.getLength();
        aOpt.maAddProps.realloc( nUsed + 2 );
        aOpt.maAddProps[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MinValue" ) );
        aOpt.maAddProps[nUsed++].Value = makeAny( i_nMinValue );
        aOpt.maAddProps[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxValue" ) );
        aOpt.maAddProps[nUsed++].Value = makeAny( i_nMaxValue );
    }

    Sequence< rtl::OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_nValue );
    Sequence< rtl::OUString > aIds(1);
    aIds[0] = i_rID;
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Range", &aVal, aOpt);
}

Any PrinterOptionsHelper::setEditControlOpt(const rtl::OUString& i_rID,
                                            const rtl::OUString& i_rTitle,
                                            const rtl::OUString& i_rHelpId,
                                            const rtl::OUString& i_rProperty,
                                            const rtl::OUString& i_rValue,
                                            const PrinterOptionsHelper::UIControlOptions& i_rControlOptions)
{
    Sequence< rtl::OUString > aHelpId;
    if( !i_rHelpId.isEmpty() )
    {
        aHelpId.realloc( 1 );
        *aHelpId.getArray() = i_rHelpId;
    }
    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_rValue );
    Sequence< rtl::OUString > aIds(1);
    aIds[0] = i_rID;
    return setUIControlOpt(aIds, i_rTitle, aHelpId, "Edit", &aVal, i_rControlOptions);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
