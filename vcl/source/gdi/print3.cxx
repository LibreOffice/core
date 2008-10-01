/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: print3.cxx,v $
 * $Revision: 1.1.2.11 $
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

#include "precompiled_vcl.hxx"

#include "vcl/print.hxx"
#include "vcl/prndlg.hxx"
#include "vcl/svapp.hxx"
#include "vcl/svdata.hxx"
#include "vcl/salinst.hxx"
#include "vcl/salprn.hxx"
#include "vcl/svids.hrc"

#include "tools/urlobj.hxx"
#include "tools/multisel.hxx"

#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "comphelper/processfactory.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace vcl;

class vcl::ImplPrinterListenerData
{
public:
    boost::shared_ptr<Printer>                                                  mpPrinter;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >       mxJobParameters;
    MultiSelection                                                              maSelection;
};

PrinterListener::PrinterListener()
    : mpImplData( new ImplPrinterListenerData )
{
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
                xFilterMgr->appendFilter( String( VclResId( SV_STDTEXT_ALLFILETYPES ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.*" ) ) );
            }
            catch( lang::IllegalArgumentException rExc )
            {
                DBG_ERRORFILE( "caught IllegalArgumentException when registering filter\n" );
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
    boost::shared_ptr<PrinterListener>  mpListener;
    JobSetup                            maInitSetup;
    Reference< beans::XPropertySet >    mxJobOptions;

    PrintJobAsync( const boost::shared_ptr<PrinterListener>& i_pListener,
                   const JobSetup& i_rInitSetup,
                   const Reference< beans::XPropertySet >& i_xJobOptions
                   )
    : mpListener( i_pListener ), maInitSetup( i_rInitSetup ), mxJobOptions( i_xJobOptions )
    {}

    DECL_LINK( ExecJob, void* );
};

IMPL_LINK( PrintJobAsync, ExecJob, void*, EMPTYARG )
{
    Printer::ImplPrintJob( mpListener, maInitSetup, mxJobOptions );

    // clean up, do not access members after this
    delete this;

    return 0;
}

void Printer::PrintJob( const boost::shared_ptr<PrinterListener>& i_pListener,
                        const JobSetup& i_rInitSetup,
                        const Reference< beans::XPropertySet >& i_xJobOptions
                        )
{
    PrintJobAsync* pAsync = new PrintJobAsync( i_pListener, i_rInitSetup, i_xJobOptions );
    Application::PostUserEvent( LINK( pAsync, PrintJobAsync, ExecJob ) );
}

void Printer::ImplPrintJob( const boost::shared_ptr<PrinterListener>& i_pListener,
                            const JobSetup& i_rInitSetup,
                            const Reference< beans::XPropertySet >& /*i_xJobOptions*/
                            )
{
    // setup printer
    boost::shared_ptr<PrinterListener> pListener( i_pListener );
    boost::shared_ptr<Printer> pPrinter( new Printer( i_rInitSetup.GetPrinterName() ) );
    pListener->setPrinter( pPrinter );

    // setup page range selection
    MultiSelection aSel;
    int nPages = i_pListener->getPageCount();
    aSel.SetTotalRange( Range( 1, nPages ) );
    aSel.SelectAll();

    // check if the printer brings up its own dialog
    // in that case leave the work to that dialog
    if( ! pListener->getPrinter()->GetCapabilities( PRINTER_CAPABILITIES_EXTERNALDIALOG ) )
    {
        try
        {
            PrintDialog aDlg( NULL, i_pListener );
            if( ! aDlg.Execute() )
                return;
            if( aDlg.isPrintToFile() )
            {
                rtl::OUString aFile = queryFile( pListener->getPrinter().get() );
                if( ! aFile.getLength() )
                    return;
                pListener->getPrinter()->EnablePrintFile( TRUE );
                pListener->getPrinter()->SetPrintFile( aFile );
            }
            aSel = aDlg.getPageSelection();
            pListener->getPrinter()->SetCopyCount( aDlg.getCopyCount(), aDlg.isCollate() );
        }
        catch( std::bad_alloc& )
        {
        }
    }

    pListener->setPageSelection( aSel );
    pListener->getPrinter()->StartJob( String( RTL_CONSTASCII_USTRINGPARAM( "FIXME: no job name" ) ),
                                       pListener );

    pListener->jobFinished();
}

bool Printer::StartJob( const XubString& i_rJobName, boost::shared_ptr<vcl::PrinterListener>& i_pListener )
{
    mnError = PRINTER_OK;

    if ( IsDisplayPrinter() )
        return FALSE;

    if ( IsJobActive() || IsPrinting() )
        return FALSE;

    if( mpPrinterData->mbNextJobIsQuick )
    {
        String aKey( RTL_CONSTASCII_USTRINGPARAM( "IsQuickJob" ) );
        if( maJobSetup.GetValue( aKey ).Len() == 0 )
            maJobSetup.SetValue( aKey, String( RTL_CONSTASCII_USTRINGPARAM( "true" ) ) );
    }

    ULONG   nCopies = mnCopyCount;
    BOOL    bCollateCopy = mbCollateCopy;
    BOOL    bUserCopy = FALSE;

    if ( nCopies > 1 )
    {
        ULONG nDevCopy;

        if ( bCollateCopy )
            nDevCopy = GetCapabilities( PRINTER_CAPABILITIES_COLLATECOPIES );
        else
            nDevCopy = GetCapabilities( PRINTER_CAPABILITIES_COPIES );

        // Muessen Kopien selber gemacht werden?
        if ( nCopies > nDevCopy )
        {
            bUserCopy = TRUE;
            nCopies = 1;
            bCollateCopy = FALSE;
        }
    }
    else
        bCollateCopy = FALSE;


    ImplSVData* pSVData = ImplGetSVData();
    mpPrinter = pSVData->mpDefInst->CreatePrinter( mpInfoPrinter );

    if ( !mpPrinter )
        return FALSE;

    XubString* pPrintFile;
    if ( mbPrintFile )
        pPrintFile = &maPrintFile;
    else
        pPrintFile = NULL;

    maJobName               = i_rJobName;
    mnCurPage               = 1;
    mnCurPrintPage          = 1;
    mbPrinting              = TRUE;
    if( ImplGetSVData()->maGDIData.mbPrinterPullModel )
    {
        mbJobActive             = TRUE;
        // sallayer does all necesseary page printing
        if( mpPrinter->StartJob( pPrintFile,
                                 Application::GetDisplayName(),
                                 maJobSetup.ImplGetConstData(),
                                 *i_pListener ) )
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
            mbPrinting          = FALSE;
            mpPrinter = NULL;

            return false;
        }
    }
    else
    {
        if( mpPrinter->StartJob( pPrintFile,
                                 i_rJobName,
                                 Application::GetDisplayName(),
                                 nCopies, bCollateCopy,
                                 maJobSetup.ImplGetConstData() ) )
        {
            mbJobActive             = TRUE;
            MultiSelection aSel( i_pListener->getPageSelection() );
            for( long nPage = aSel.FirstSelected(); nPage != long(SFX_ENDOFSELECTION); nPage = aSel.NextSelected() )
            {
                // remember MultiSelection is 1 based (due to user input)
                i_pListener->printFilteredPage( static_cast<int>(nPage-1) );
            }
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
            mbPrinting          = FALSE;
            mpPrinter = NULL;

            return false;
        }
    }

    return true;
}

PrinterListener::~PrinterListener()
{
    delete mpImplData;
}

const boost::shared_ptr<Printer>& PrinterListener::getPrinter() const
{
    return mpImplData->mpPrinter;
}

const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& PrinterListener::getJobParameters() const
{
    return mpImplData->mxJobParameters;
}

const MultiSelection& PrinterListener::getPageSelection() const
{
    return mpImplData->maSelection;
}

void PrinterListener::setPrinter( const boost::shared_ptr<Printer>& i_rPrinter )
{
    mpImplData->mpPrinter = i_rPrinter;
}

void PrinterListener::setJobParameters( const Reference< beans::XPropertySet >& i_pParams )
{
    mpImplData->mxJobParameters = i_pParams;
}

void PrinterListener::setPageSelection( const MultiSelection& i_rSel )
{
    mpImplData->maSelection = i_rSel;
}

void PrinterListener::printFilteredPage( int i_nPage )
{
    // get page parameters
    JobSetup aPageSetup;
    Size aPageSize;
    getPageParameters( i_nPage, aPageSetup, aPageSize );
    const MapMode aMapMode( MAP_100TH_MM );


    mpImplData->mpPrinter->Push();
    mpImplData->mpPrinter->EnableOutput( FALSE );
    mpImplData->mpPrinter->SetMapMode( aMapMode );

    GDIMetaFile aMtf;
    aMtf.Record( mpImplData->mpPrinter.get() );

    printPage( i_nPage );

    aMtf.Stop();
    aMtf.WindStart();
    mpImplData->mpPrinter->Pop();

    // FIXME: do transparency filtering here when vcl92 is integrated

    mpImplData->mpPrinter->EnableOutput( TRUE );
    if( aPageSetup != mpImplData->mpPrinter->GetJobSetup() ) // set new setup if changed
        mpImplData->mpPrinter->SetJobSetup( aPageSetup );

    // actually print the page
    mpImplData->mpPrinter->StartPage();

    mpImplData->mpPrinter->Push();
    mpImplData->mpPrinter->SetMapMode( MAP_100TH_MM );
    aMtf.WindStart();
    aMtf.Play( mpImplData->mpPrinter.get() );
    mpImplData->mpPrinter->Pop();

    mpImplData->mpPrinter->EndPage();
}

void PrinterListener::setListeners()
{
}

void PrinterListener::jobFinished()
{
}

