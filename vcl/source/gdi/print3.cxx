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

#include <sal/types.h>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>

#include <vcl/metaact.hxx>
#include <vcl/print.hxx>
#include <vcl/printer/PrinterController.hxx>
#include <vcl/printer/PrinterOptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <configsettings.hxx>
#include <printdlg.hxx>
#include <salinst.hxx>
#include <salprn.hxx>
#include <strings.hrc>
#include <svdata.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/view/DuplexMode.hpp>

#include <unordered_map>
#include <unordered_set>

using namespace vcl;

namespace {

struct PrintJobAsync
{
    std::shared_ptr<vcl::print::PrinterController>  mxController;
    JobSetup                            maInitSetup;

    PrintJobAsync(const std::shared_ptr<vcl::print::PrinterController>& i_xController,
                  const JobSetup& i_rInitSetup)
    : mxController( i_xController ), maInitSetup( i_rInitSetup )
    {}

    DECL_LINK( ExecJob, void*, void );
};

}
static OUString queryFile( Printer const * pPrinter )
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
#elif defined _WIN32
        (void)pPrinter;
        xFilePicker->appendFilter( "*.PRN", "*.prn" );
#endif
        // add arbitrary files
        xFilePicker->appendFilter(VclResId(SV_STDTEXT_ALLFILETYPES), "*.*");
    }
    catch (const css::lang::IllegalArgumentException&)
    {
        TOOLS_WARN_EXCEPTION( "vcl.gdi", "caught IllegalArgumentException when registering filter" );
    }

    if( xFilePicker->execute() == css::ui::dialogs::ExecutableDialogResults::OK )
    {
        css::uno::Sequence< OUString > aPathSeq( xFilePicker->getSelectedFiles() );
        INetURLObject aObj( aPathSeq[0] );
        aResult = aObj.PathToFileName();
    }
    return aResult;
}

IMPL_LINK_NOARG(PrintJobAsync, ExecJob, void*, void)
{
    Printer::ImplPrintJob(mxController, maInitSetup);

    // clean up, do not access members after this
    delete this;
}

void Printer::PrintJob(const std::shared_ptr<print::PrinterController>& i_xController,
                       const JobSetup& i_rInitSetup)
{
    bool bSynchronous = false;
    css::beans::PropertyValue* pVal = i_xController->getValue( "Wait" );
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

bool Printer::PreparePrintJob(std::shared_ptr<print::PrinterController> xController,
                           const JobSetup& i_rInitSetup)
{
    // check if there is a default printer; if not, show an error box (if appropriate)
    if( GetDefaultPrinterName().isEmpty() )
    {
        if (xController->isShowDialogs())
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(xController->getWindow(), "vcl/ui/errornoprinterdialog.ui"));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("ErrorNoPrinterDialog"));
            xBox->run();
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
        xController->setPapersizeFromSetup(xPrinter->GetPrinterSettingsPreferred());
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
    css::beans::PropertyValue* pContentVal = xController->getValue("PrintRange");
    if( ! pContentVal )
        pContentVal = xController->getValue("PrintContent");

    // case 1: UNO API has set "Pages"
    css::beans::PropertyValue* pPagesVal = xController->getValue("Pages");
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
                pContentVal->Value <<= sal_Int32( 1 );
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
                css::beans::PropertyValue* pRangeVal = xController->getValue("PageRange");
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

    css::beans::PropertyValue* pReverseVal = xController->getValue("PrintReverse");
    if( pReverseVal )
    {
        bool bReverse = false;
        pReverseVal->Value >>= bReverse;
        xController->setReversePrint( bReverse );
    }

    css::beans::PropertyValue* pPapersizeFromSetupVal = xController->getValue("PapersizeFromSetup");
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
        print::PrinterController::MultiPageSetup aMPS;
        aMPS.nRows         = std::max<sal_Int32>(nRows, 1);
        aMPS.nColumns      = std::max<sal_Int32>(nCols, 1);
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
        aMPS.nOrder = static_cast<print::NupOrderType>(xController->getIntProperty( "NUpSubPageOrder", static_cast<sal_Int32>(aMPS.nOrder) ));
        aMPS.aPaperSize = xController->getPrinter()->PixelToLogic( xController->getPrinter()->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) );
        css::beans::PropertyValue* pPgSizeVal = xController->getValue( "NUpPaperSize" );
        css::awt::Size aSizeVal;
        if( pPgSizeVal && (pPgSizeVal->Value >>= aSizeVal) )
        {
            aMPS.aPaperSize.setWidth( aSizeVal.Width );
            aMPS.aPaperSize.setHeight( aSizeVal.Height );
        }

        xController->setMultipage( aMPS );
    }

    // in direct print case check whether there is anything to print.
    // if not, show an errorbox (if appropriate)
    if( xController->isShowDialogs() && xController->isDirectPrint() )
    {
        if( xController->getFilteredPageCount() == 0 )
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(xController->getWindow(), "vcl/ui/errornocontentdialog.ui"));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("ErrorNoContentDialog"));
            xBox->run();
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
            PrintDialog aDlg(xController->getWindow(), xController);
            if (!aDlg.run())
            {
                xController->abortJob();
                return false;
            }
            if (aDlg.isPrintToFile())
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
            else if (aDlg.isSingleJobs())
            {
                xController->getPrinter()->SetSinglePrintJobs(true);
            }
        }
        catch (const std::bad_alloc&)
        {
        }
    }

    xController->pushPropertiesToPrinter();
    return true;
}

bool Printer::ExecutePrintJob(const std::shared_ptr<print::PrinterController>& xController)
{
    OUString aJobName;
    css::beans::PropertyValue* pJobNameVal = xController->getValue( "JobName" );
    if( pJobNameVal )
        pJobNameVal->Value >>= aJobName;

    return xController->getPrinter()->StartJob( aJobName, xController );
}

void Printer::FinishPrintJob(const std::shared_ptr<print::PrinterController>& xController)
{
    xController->resetPaperToLastConfigured();
    xController->jobFinished( xController->getJobState() );
}

void Printer::ImplPrintJob(const std::shared_ptr<print::PrinterController>& xController,
                           const JobSetup& i_rInitSetup)
{
    if (PreparePrintJob(xController, i_rInitSetup))
    {
        ExecutePrintJob(xController);
    }
    FinishPrintJob(xController);
}

bool Printer::StartJob( const OUString& i_rJobName, std::shared_ptr<vcl::print::PrinterController> const & i_xController)
{
    mnError = ERRCODE_NONE;

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

    bool bSinglePrintJobs = i_xController->getPrinter()->IsSinglePrintJobs();

    css::beans::PropertyValue* pFileValue = i_xController->getValue("LocalFileName");
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
            mbPrinting = false;
            mpPrinter.reset();
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
                        mbPrinting              = true;
                    else
                        bError = true;
                }
            }
            else
                bError = true;

            if( bError )
            {
                mnError = mpPrinter ? ImplSalPrinterErrorCodeToVCL(mpPrinter->GetErrorCode()) : ERRCODE_NONE;
                if ( !mnError )
                    mnError = PRINTER_GENERALERROR;
                i_xController->setJobState( mnError == PRINTER_ABORT
                                            ? css::view::PrintableState_JOB_ABORTED
                                            : css::view::PrintableState_JOB_FAILED );
                mbPrinting = false;
                mpPrinter.reset();

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
