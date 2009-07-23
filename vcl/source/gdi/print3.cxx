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
#include "vcl/metaact.hxx"

#include "tools/urlobj.hxx"

#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/awt/Size.hpp"
#include "comphelper/processfactory.hxx"

#include <hash_map>
#include <hash_set>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace vcl;

class ImplPageCache
{
    std::vector< GDIMetaFile >  maPages;
    std::vector< sal_Int32 >    maPageNumbers;
    std::vector< sal_Int32 >    maCacheRanking;

    static const sal_Int32 nCacheSize = 6;

    void updateRanking( sal_Int32 nLastHit )
    {
        if( maCacheRanking[0] != nLastHit )
        {
            bool bMove = false;
            for( sal_Int32 i = nCacheSize-1; i > 0; i-- )
            {
                if( maCacheRanking[i] == nLastHit )
                    bMove = true;
                maCacheRanking[i] = maCacheRanking[i-1];
            }
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
    void insert( sal_Int32 i_nPageNo, const GDIMetaFile& i_rPage )
    {
        sal_Int32 nReplacePage = maCacheRanking.back();
        maPages[ nReplacePage ] = i_rPage;
        maPageNumbers[ nReplacePage ] = i_nPageNo;
        // cache insertion means in our case, the page was just queried
        // so update the ranking
        updateRanking( nReplacePage );
    }

    // caution: bad algorithm; should there ever be reason to increase the cache size beyond 6
    // this needs to be urgently rewritten. However do NOT increase the cache size lightly,
    // whole pages can be rather memory intensive
    const GDIMetaFile* get( sal_Int32 i_nPageNo )
    {
        const GDIMetaFile* pRet = NULL;
        for( sal_Int32 i = 0; i < nCacheSize; ++i )
        {
            if( maPageNumbers[i] == i_nPageNo )
            {
                updateRanking( i );
                pRet = &maPages[i];
                break;
            }
        }
        return pRet;
    }

    void invalidate()
    {
        for( sal_Int32 i = 0; i < nCacheSize; ++i )
        {
            maPageNumbers[i] = -1;
            maPages[i].Clear();
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

    typedef std::hash_map< rtl::OUString, size_t, rtl::OUStringHash > PropertyToIndexMap;
    typedef std::hash_map< rtl::OUString, ControlDependency, rtl::OUStringHash > ControlDependencyMap;

    boost::shared_ptr<Printer>                                  mpPrinter;
    Sequence< PropertyValue >                                   maUIOptions;
    std::vector< PropertyValue >                                maUIProperties;
    std::vector< bool >                                         maUIPropertyEnabled;
    PropertyToIndexMap                                          maPropertyToIndex;
    Link                                                        maOptionChangeHdl;
    ControlDependencyMap                                        maControlDependencies;
    sal_Bool                                                    mbLastPage;
    view::PrintableState                                        meJobState;

    vcl::PrinterController::MultiPageSetup                      maMultiPage;

    vcl::PrintProgressDialog*                                   mpProgress;

    ImplPageCache                                               maPageCache;

    ImplPrinterControllerData() :
        mbLastPage( sal_False ),
        meJobState( view::PrintableState_JOB_STARTED ),
        mpProgress( NULL )
    {}
    ~ImplPrinterControllerData() { delete mpProgress; }
};

PrinterController::PrinterController()
    : mpImplData( new ImplPrinterControllerData )
{
}

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
    boost::shared_ptr<PrinterController>  mpController;
    JobSetup                            maInitSetup;

    PrintJobAsync( const boost::shared_ptr<PrinterController>& i_pController,
                   const JobSetup& i_rInitSetup
                   )
    : mpController( i_pController ), maInitSetup( i_rInitSetup )
    {}

    DECL_LINK( ExecJob, void* );
};

IMPL_LINK( PrintJobAsync, ExecJob, void*, EMPTYARG )
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
    beans::PropertyValue* pVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) ) );
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
    // setup printer
    boost::shared_ptr<PrinterController> pController( i_pController );
    // if no specific printer is already set, create one
    if( ! pController->getPrinter() )
    {
        boost::shared_ptr<Printer> pPrinter( new Printer( i_rInitSetup.GetPrinterName() ) );
        pController->setPrinter( pPrinter );
    }

    // reset last page property
    i_pController->setLastPage( sal_False );

    beans::PropertyValue* pPagesVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Pages" ) ) );
    if( pPagesVal )
    {
        rtl::OUString aPagesVal;
        pPagesVal->Value >>= aPagesVal;
        if( aPagesVal.getLength() )
        {
            // "Pages" attribute from API is now equivalent to "PageRange"
            // AND "PrintContent" = 1 except calc where it is "PrintRange" = 1
            // Argh ! That sure needs cleaning up
            beans::PropertyValue* pVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) ) );
            if( ! pVal )
                pVal = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintContent" ) ) );
            if( pVal )
            {
                pVal->Value = makeAny( sal_Int32( 1 ) );
                i_pController->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ), pPagesVal->Value );
            }
        }
    }

    // check if the printer brings up its own dialog
    // in that case leave the work to that dialog
    const String& rQuick( i_rInitSetup.GetValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsQuickJob" ) ) ) );
    bool bIsQuick = rQuick.Len() && rQuick.EqualsIgnoreCaseAscii( "true" );
    if( ! pController->getPrinter()->GetCapabilities( PRINTER_CAPABILITIES_EXTERNALDIALOG ) && ! bIsQuick )
    {
        try
        {
            PrintDialog aDlg( NULL, i_pController );
            if( ! aDlg.Execute() )
            {
                GDIMetaFile aPageFile;
                i_pController->setLastPage( sal_True );
                if( i_pController->getPageCount() > 0 )
                    i_pController->getFilteredPageFile( 0, aPageFile );
                return;
            }
            if( aDlg.isPrintToFile() )
            {
                rtl::OUString aFile = queryFile( pController->getPrinter().get() );
                if( ! aFile.getLength() )
                    return;
                pController->setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LocalFileName" ) ),
                                     makeAny( aFile ) );
            }
        }
        catch( std::bad_alloc& )
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
        return FALSE;

    if ( IsJobActive() || IsPrinting() )
        return FALSE;

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

        // need to do copies by hand ?
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

    // remark: currently it is still possible to use EnablePrintFile and
    // SetPrintFileName to redirect printout into file
    // it can be argued that those methods should be removed in favor
    // of only using the LocalFileName property
    beans::PropertyValue* pFileValue = i_pController->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LocalFileName" ) ) );
    if( pFileValue )
    {
        rtl::OUString aFile;
        pFileValue->Value >>= aFile;
        if( aFile.getLength() )
        {
            mbPrintFile = TRUE;
            maPrintFile = aFile;
        }
    }

    XubString* pPrintFile = NULL;
    if ( mbPrintFile )
        pPrintFile = &maPrintFile;

    maJobName               = i_rJobName;
    mnCurPage               = 1;
    mnCurPrintPage          = 1;
    mbPrinting              = TRUE;
    if( ImplGetSVData()->maGDIData.mbPrinterPullModel )
    {
        mbJobActive             = TRUE;
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
            mbPrinting          = FALSE;
            mpPrinter = NULL;

            return false;
        }
    }
    else
    {
        // possibly a dialog has been shown
        // now the real job starts
        i_pController->setJobState( view::PrintableState_JOB_STARTED );
        i_pController->jobStarted();

        if( mpPrinter->StartJob( pPrintFile,
                                 i_rJobName,
                                 Application::GetDisplayName(),
                                 nCopies, bCollateCopy,
                                 maJobSetup.ImplGetConstData() ) )
        {
            mbJobActive             = TRUE;
            i_pController->createProgressDialog();
            int nPages = i_pController->getFilteredPageCount();
            int nRepeatCount = bUserCopy ? mnCopyCount : 1;
            for( int nIteration = 0; nIteration < nRepeatCount; nIteration++ )
            {
                for( int nPage = 0; nPage < nPages; nPage++ )
                {
                    if( nPage == nPages-1 && nIteration == nRepeatCount-1 )
                        i_pController->setLastPage( sal_True );
                    i_pController->printFilteredPage( nPage );
                }
                // FIXME: duplex ?
            }
            EndJob();

            if( i_pController->getJobState() == view::PrintableState_JOB_STARTED )
                i_pController->setJobState( view::PrintableState_JOB_SPOOLED );
        }
        else
        {
            i_pController->setJobState( view::PrintableState_JOB_FAILED );

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
    Size aPaperSize( i_rPrinter->PixelToLogic( i_rPrinter->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );
    setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ),
              makeAny( rtl::OUString( i_rPrinter->GetName() ) ) );
}

static Size modifyJobSetup( Printer* pPrinter, const Sequence< PropertyValue >& i_rProps )
{
    Size aPageSize = pPrinter->GetPaperSize();
    for( sal_Int32 nProperty = 0, nPropertyCount = i_rProps.getLength(); nProperty < nPropertyCount; ++nProperty )
    {
        if( i_rProps[ nProperty ].Name.equalsAscii( "PageSize" ) )
        {
            awt::Size aSize;
            i_rProps[ nProperty].Value >>= aSize;
            aPageSize.Width() = aSize.Width;
            aPageSize.Height() = aSize.Height;

            Size aCurSize( pPrinter->GetPaperSize() );
            if( aPageSize != aCurSize )
                pPrinter->SetPaperSizeUser( aPageSize );
        }
    }
    return aPageSize;
}

Size PrinterController::getPageFile( int i_nUnfilteredPage, GDIMetaFile& o_rMtf, bool i_bMayUseCache )
{
    // update progress if necessary
    if( mpImplData->mpProgress )
    {
        // do nothing if printing is canceled
        if( mpImplData->mpProgress->isCanceled() )
           return Size();
        mpImplData->mpProgress->tick();
        Application::Reschedule( true );
    }

    if( i_bMayUseCache )
    {
        const GDIMetaFile* pCached = mpImplData->maPageCache.get( i_nUnfilteredPage );
        if( pCached )
        {
            o_rMtf = *pCached;
            return pCached->GetPrefSize();
        }
    }
    else
        mpImplData->maPageCache.invalidate();

    o_rMtf.Clear();

    // get page parameters
    Sequence< PropertyValue > aPageParm( getPageParameters( i_nUnfilteredPage ) );
    const MapMode aMapMode( MAP_100TH_MM );

    mpImplData->mpPrinter->Push();
    mpImplData->mpPrinter->SetMapMode( aMapMode );

    // modify job setup if necessary
    Size aPageSize = modifyJobSetup( mpImplData->mpPrinter.get(), aPageParm );

    o_rMtf.SetPrefSize( aPageSize );
    o_rMtf.SetPrefMapMode( aMapMode );

    mpImplData->mpPrinter->EnableOutput( FALSE );

    o_rMtf.Record( mpImplData->mpPrinter.get() );

    printPage( i_nUnfilteredPage );

    o_rMtf.Stop();
    o_rMtf.WindStart();
    mpImplData->mpPrinter->Pop();

    if( i_bMayUseCache )
        mpImplData->maPageCache.insert( i_nUnfilteredPage, o_rMtf );

    return aPageSize;
}

static void appendSubPage( GDIMetaFile& o_rMtf, const Rectangle& i_rClipRect, GDIMetaFile& io_rSubPage, bool i_bDrawBorder )
{
    // intersect all clipregion actions with our clip rect
    io_rSubPage.WindStart();
    io_rSubPage.Clip( i_rClipRect );

    // save gstate
    o_rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR | PUSH_FILLCOLOR | PUSH_CLIPREGION | PUSH_MAPMODE ) );

    // draw a border
    if( i_bDrawBorder )
    {
        Rectangle aBorderRect( i_rClipRect );
        aBorderRect.Left()   -= 100;
        aBorderRect.Top()    -= 100;
        aBorderRect.Right()  += 100;
        aBorderRect.Bottom() += 100;
        o_rMtf.AddAction( new MetaLineColorAction( Color( COL_BLACK ), TRUE ) );
        o_rMtf.AddAction( new MetaFillColorAction( Color( COL_TRANSPARENT ), FALSE ) );
        o_rMtf.AddAction( new MetaRectAction( aBorderRect ) );
    }

    // clip to page rect
    o_rMtf.AddAction( new MetaClipRegionAction( Region( i_rClipRect ), TRUE ) );

    // append the subpage
    io_rSubPage.WindStart();
    io_rSubPage.Play( o_rMtf );

    // restore gstate
    o_rMtf.AddAction( new MetaPopAction() );
}

Size PrinterController::getFilteredPageFile( int i_nFilteredPage, GDIMetaFile& o_rMtf, bool i_bMayUseCache )
{
    const MultiPageSetup& rMPS( mpImplData->maMultiPage );
    int nSubPages = rMPS.nRows * rMPS.nColumns;
    if( nSubPages < 1 )
        nSubPages = 1;

    // there is no filtering to be done (and especially the page size of the
    // original page is to be set), when N-Up is "neutral" that is there is
    // only one subpage and the margins are 0
    if( nSubPages == 1 &&
        rMPS.nLeftMargin == 0 && rMPS.nRightMargin == 0 &&
        rMPS.nTopMargin == 0 && rMPS.nBottomMargin == 0 )
    {
        return getPageFile( i_nFilteredPage, o_rMtf, i_bMayUseCache );
    }

    Size aPaperSize( mpImplData->maMultiPage.aPaperSize );
    // multi page area: paper size minus margins + one time spacing right and down
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

    int nDocPages = getPageCount();
    for( int nSubPage = 0; nSubPage < nSubPages; nSubPage++ )
    {
        // map current sub page to real page
        int nPage = (i_nFilteredPage * nSubPages + nSubPage) / rMPS.nRepeat;
        if( nPage < nDocPages )
        {
            GDIMetaFile aPageFile;
            Size aPageSize = getPageFile( nPage, aPageFile, i_bMayUseCache );
            if( aPageSize.Width() && aPageSize.Height() )
            {
                // scale the metafile down to a sub page size
                double fScaleX = double(aSubPageSize.Width())/double(aPageSize.Width());
                double fScaleY = double(aSubPageSize.Height())/double(aPageSize.Height());
                double fScale  = std::min( fScaleX, fScaleY );
                aPageFile.Scale( fScale, fScale );
                aPageFile.WindStart();

                // move the subpage so it is centered in its "cell"
                long nOffX = (aSubPageSize.Width() - long(double(aPageSize.Width()) * fScale)) / 2;
                long nOffY = (aSubPageSize.Height() - long(double(aPageSize.Height()) * fScale)) / 2;
                long nX = rMPS.nLeftMargin + nOffX + nAdvX * (nSubPage % rMPS.nColumns);
                long nY = rMPS.nTopMargin + nOffY + nAdvY * (nSubPage / rMPS.nColumns);
                aPageFile.Move( nX, nY );
                aPageFile.WindStart();
                // calculate border rectangle
                Rectangle aSubPageRect( Point( nX, nY ),
                                        Size( long(double(aPageSize.Width())*fScale),
                                              long(double(aPageSize.Height())*fScale) ) );

                // append subpage to page
                appendSubPage( o_rMtf, aSubPageRect, aPageFile, rMPS.bDrawBorder );
            }
        }
    }
    o_rMtf.WindStart();

    return aPaperSize;
}

int PrinterController::getFilteredPageCount()
{
    int nDiv = mpImplData->maMultiPage.nRows * mpImplData->maMultiPage.nColumns;
    if( nDiv < 1 )
        nDiv = 1;
    return (getPageCount() * mpImplData->maMultiPage.nRepeat + (nDiv-1)) / nDiv;
}

void PrinterController::printFilteredPage( int i_nPage )
{
    if( mpImplData->meJobState != view::PrintableState_JOB_STARTED )
        return;

    GDIMetaFile aPageFile;
    Size aPageSize = getFilteredPageFile( i_nPage, aPageFile );

    if( mpImplData->mpProgress )
    {
        // do nothing if printing is canceled
        if( mpImplData->mpProgress->isCanceled() )
        {
            setJobState( view::PrintableState_JOB_ABORTED );
            return;
        }
    }

    bool bMultiPageOutput = mpImplData->maMultiPage.nRows != 1 || mpImplData->maMultiPage.nColumns != 1;
    ULONG nRestoreDrawMode = mpImplData->mpPrinter->GetDrawMode();
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

    GDIMetaFile aCleanedFile;
    mpImplData->mpPrinter->RemoveTransparenciesFromMetaFile( aPageFile, aCleanedFile, nMaxBmpDPIX, nMaxBmpDPIY,
                                                             rPrinterOptions.IsReduceTransparency(),
                                                             rPrinterOptions.GetReducedTransparencyMode() == PRINTER_TRANSPARENCY_AUTO,
                                                             rPrinterOptions.IsReduceBitmaps() && rPrinterOptions.IsReducedBitmapIncludesTransparency()
                                                             );

    mpImplData->mpPrinter->EnableOutput( TRUE );

    // in N-Up printing set the correct page size
    mpImplData->mpPrinter->SetMapMode( MAP_100TH_MM );
    if( bMultiPageOutput )
        mpImplData->mpPrinter->SetPaperSizeUser( aPageSize = mpImplData->maMultiPage.aPaperSize );

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
}

void PrinterController::setLastPage( sal_Bool i_bLastPage )
{
    mpImplData->mbLastPage = i_bLastPage;
}

Sequence< PropertyValue > PrinterController::getJobProperties( const Sequence< PropertyValue >& i_rMergeList ) const
{
    std::hash_set< rtl::OUString, rtl::OUStringHash > aMergeSet;
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
    std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    return it != mpImplData->maPropertyToIndex.end() ? &mpImplData->maUIProperties[it->second] : NULL;
}

const beans::PropertyValue* PrinterController::getValue( const rtl::OUString& i_rProperty ) const
{
    std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    return it != mpImplData->maPropertyToIndex.end() ? &mpImplData->maUIProperties[it->second] : NULL;
}

Sequence< beans::PropertyValue > PrinterController::getValues( const Sequence< rtl::OUString >& i_rNames ) const
{
    Sequence< beans::PropertyValue > aRet( i_rNames.getLength() );
    sal_Int32 nFound = 0;
    for( sal_Int32 i = 0; i < i_rNames.getLength(); i++ )
    {
        const beans::PropertyValue* pVal = getValue( i_rNames[i] );
        if( pVal )
            aRet[ nFound++ ] = *pVal;
    }
    aRet.realloc( nFound );
    return aRet;
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
    std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
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
        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if( rEntry.Name.equalsAscii( "Property" ) )
            {
                PropertyValue aVal;
                rEntry.Value >>= aVal;
                DBG_ASSERT( mpImplData->maPropertyToIndex.find( aVal.Name )
                            == mpImplData->maPropertyToIndex.end(), "duplicate property entry" );
                setValue( aVal );
                aPropName = aVal.Name;
                bHaveProperty = true;
            }
            else if( rEntry.Name.equalsAscii( "Enabled" ) )
            {
                sal_Bool bValue = sal_True;
                rEntry.Value >>= bValue;
                bIsEnabled = bValue;
            }
            else if( rEntry.Name.equalsAscii( "DependsOnName" ) )
            {
                rEntry.Value >>= aDep.maDependsOnName;
            }
            else if( rEntry.Name.equalsAscii( "DependsOnEntry" ) )
            {
                rEntry.Value >>= aDep.mnDependsOnEntry;
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
            if( aDep.maDependsOnName.getLength() > 0 )
                mpImplData->maControlDependencies[ aPropName ] = aDep;
        }
    }
}

void PrinterController::enableUIOption( const rtl::OUString& i_rProperty, bool i_bEnable )
{
    std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    if( it != mpImplData->maPropertyToIndex.end() )
    {
        // call handler only for actual changes
        if( ( mpImplData->maUIPropertyEnabled[ it->second ] && ! i_bEnable ) ||
            ( ! mpImplData->maUIPropertyEnabled[ it->second ] && i_bEnable ) )
        {
            mpImplData->maUIPropertyEnabled[ it->second ] = i_bEnable;
            rtl::OUString aPropName( i_rProperty );
            mpImplData->maOptionChangeHdl.Call( &aPropName );
        }
    }
}

bool PrinterController::isUIOptionEnabled( const rtl::OUString& i_rProperty ) const
{
    bool bEnabled = false;
    std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator prop_it =
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
                            OSL_ENSURE( 0, "strange type in control dependency" );
                            bEnabled = false;
                        }
                    }
                }
            }
        }
    }
    return bEnabled;
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

        if( bShow )
        {
            mpImplData->mpProgress = new PrintProgressDialog( NULL, getPageCount() );
            mpImplData->mpProgress->Show();
        }
    }
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
    mpImplData->mpPrinter->SetCopyCount( static_cast<USHORT>(nCopyCount), bCollate );
}

/*
 * PrinterOptionsHelper
**/
Any PrinterOptionsHelper::getValue( const rtl::OUString& i_rPropertyName ) const
{
    Any aRet;
    std::hash_map< rtl::OUString, Any, rtl::OUStringHash >::const_iterator it =
        m_aPropertyMap.find( i_rPropertyName );
    if( it != m_aPropertyMap.end() )
        aRet = it->second;
    return aRet;
}

bool PrinterOptionsHelper::hasProperty( const rtl::OUString& i_rPropertyName ) const
{
    Any aRet;
    std::hash_map< rtl::OUString, Any, rtl::OUStringHash >::const_iterator it =
        m_aPropertyMap.find( i_rPropertyName );
    return it != m_aPropertyMap.end();
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
        std::hash_map< rtl::OUString, Any, rtl::OUStringHash >::iterator it =
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

Any PrinterOptionsHelper::getUIControlOpt( const rtl::OUString& i_rTitle,
                                           const Sequence< rtl::OUString >& i_rHelpTexts,
                                           const rtl::OUString& i_rType,
                                           const PropertyValue* i_pVal,
                                           const rtl::OUString* i_pDependsOnName,
                                           sal_Int32 i_nDependsOnEntry,
                                           sal_Bool i_bAttachToDependency,
                                           const PropertyValue* i_pAddProps,
                                           sal_Int32 i_nAddProps
                                           )
{
    sal_Int32 nElements =
        1                                                               // ControlType
        + (i_rTitle.getLength() ? 1 : 0)                                // Text
        + (i_rHelpTexts.getLength() ? 1 : 0)                            // HelpText
        + (i_pVal ? 1 : 0)                                              // Property
        + (i_pDependsOnName ? (i_nDependsOnEntry != -1 ? 2 : 1) : 0)    // dependencies
        + (i_pDependsOnName && i_bAttachToDependency ? 1 : 0)           // attach to dependency
        + i_nAddProps
        ;

    Sequence< PropertyValue > aCtrl( nElements );
    sal_Int32 nUsed = 0;
    if( i_rTitle.getLength() )
    {
        aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ) );
        aCtrl[nUsed++].Value = makeAny( i_rTitle );
    }
    if( i_rHelpTexts.getLength() )
    {
        aCtrl[nUsed  ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HelpText" ) );
        aCtrl[nUsed++].Value = makeAny( i_rHelpTexts );
    }
    aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ControlType" ) );
    aCtrl[nUsed++].Value = makeAny( i_rType );
    if( i_pVal )
    {
        aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Property" ) );
        aCtrl[nUsed++].Value = makeAny( *i_pVal );
    }
    if( i_pDependsOnName )
    {
        aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DependsOnName" ) );
        aCtrl[nUsed++].Value = makeAny( *i_pDependsOnName );
        if( i_nDependsOnEntry != -1 )
        {
            aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DependsOnEntry" ) );
            aCtrl[nUsed++].Value = makeAny( i_nDependsOnEntry );
        }
        if( i_bAttachToDependency )
        {
            aCtrl[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AttachToDependency" ) );
            aCtrl[nUsed++].Value = makeAny( i_bAttachToDependency );
        }
    }
    for( sal_Int32 i = 0; i < i_nAddProps; i++ )
        aCtrl[ nUsed++ ] = i_pAddProps[i];

    DBG_ASSERT( nUsed == nElements, "nUsed != nElements, probable heap corruption" );

    return makeAny( aCtrl );
}

Any PrinterOptionsHelper::getGroupControlOpt( const rtl::OUString& i_rTitle, const rtl::OUString& i_rHelpText )
{
    Sequence< rtl::OUString > aHelpText;
    if( i_rHelpText.getLength() > 0 )
    {
        aHelpText.realloc( 1 );
        *aHelpText.getArray() = i_rHelpText;
    }
    return getUIControlOpt( i_rTitle, aHelpText, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Group" ) ) );
}

Any PrinterOptionsHelper::getSubgroupControlOpt( const rtl::OUString& i_rTitle, const rtl::OUString& i_rHelpText,
                                                 const rtl::OUString& i_rGroupHint, bool i_bInternalOnly )
{
    PropertyValue aAddProps[2];
    sal_Int32 nUsed = 0;
    if( i_rGroupHint.getLength() )
    {
        aAddProps[nUsed  ].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GroupingHint" ) );
        aAddProps[nUsed++].Value <<= i_rGroupHint;
    }
    if( i_bInternalOnly )
    {
        aAddProps[nUsed  ].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InternalUIOnly" ) );
        aAddProps[nUsed++].Value <<= sal_True;
    }
    Sequence< rtl::OUString > aHelpText;
    if( i_rHelpText.getLength() > 0 )
    {
        aHelpText.realloc( 1 );
        *aHelpText.getArray() = i_rHelpText;
    }
    return getUIControlOpt( i_rTitle, aHelpText, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Subgroup" ) ),
                            NULL, NULL, -1, sal_False, aAddProps, nUsed );
}

Any PrinterOptionsHelper::getBoolControlOpt( const rtl::OUString& i_rTitle,
                                             const rtl::OUString& i_rHelpText,
                                             const rtl::OUString& i_rProperty,
                                             sal_Bool i_bValue,
                                             const rtl::OUString* i_pDependsOnName,
                                             sal_Int32 i_nDependsOnEntry,
                                             sal_Bool i_bAttachToDependency
                                             )
{
    Sequence< rtl::OUString > aHelpText;
    if( i_rHelpText.getLength() > 0 )
    {
        aHelpText.realloc( 1 );
        *aHelpText.getArray() = i_rHelpText;
    }
    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_bValue );
    return getUIControlOpt( i_rTitle, aHelpText, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Bool" ) ), &aVal, i_pDependsOnName, i_nDependsOnEntry, i_bAttachToDependency );
}

Any PrinterOptionsHelper::getChoiceControlOpt( const rtl::OUString& i_rTitle,
                                               const Sequence< rtl::OUString >& i_rHelpText,
                                               const rtl::OUString& i_rProperty,
                                               const Sequence< rtl::OUString >& i_rChoices,
                                               sal_Int32 i_nValue,
                                               const rtl::OUString& i_rType,
                                               const rtl::OUString* i_pDependsOnName,
                                               sal_Int32 i_nDependsOnEntry,
                                               sal_Bool i_bAttachToDependency
                                               )
{
    PropertyValue aAddProps[2];
    sal_Int32 nUsed = 0;
    aAddProps[nUsed  ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Choices" ) );
    aAddProps[nUsed++].Value = makeAny( i_rChoices );

    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_nValue );
    return getUIControlOpt( i_rTitle, i_rHelpText, i_rType, &aVal, i_pDependsOnName, i_nDependsOnEntry, i_bAttachToDependency, aAddProps, nUsed );
}

Any PrinterOptionsHelper::getRangeControlOpt( const rtl::OUString& i_rTitle,
                                              const rtl::OUString& i_rHelpText,
                                              const rtl::OUString& i_rProperty,
                                              sal_Int32 i_nValue,
                                              sal_Int32 i_nMinValue,
                                              sal_Int32 i_nMaxValue,
                                              const rtl::OUString* i_pDependsOnName,
                                              sal_Int32 i_nDependsOnEntry,
                                              sal_Bool i_bAttachToDependency
                                            )
{
    PropertyValue aAddProps[2];
    sal_Int32 nUsed = 0;
    if( i_nMaxValue >= i_nMinValue )
    {
        aAddProps[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MinValue" ) );
        aAddProps[nUsed++].Value = makeAny( i_nMinValue );
        aAddProps[nUsed  ].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxValue" ) );
        aAddProps[nUsed++].Value = makeAny( i_nMaxValue );
    }

    Sequence< rtl::OUString > aHelpText;
    if( i_rHelpText.getLength() > 0 )
    {
        aHelpText.realloc( 1 );
        *aHelpText.getArray() = i_rHelpText;
    }
    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_nValue );
    return getUIControlOpt( i_rTitle,
                            aHelpText,
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Range" ) ),
                            &aVal,
                            i_pDependsOnName,
                            i_nDependsOnEntry,
                            i_bAttachToDependency,
                            aAddProps, nUsed );
}

Any PrinterOptionsHelper::getEditControlOpt( const rtl::OUString& i_rTitle,
                                             const rtl::OUString& i_rHelpText,
                                             const rtl::OUString& i_rProperty,
                                             const rtl::OUString& i_rValue,
                                             const rtl::OUString* i_pDependsOnName,
                                             sal_Int32 i_nDependsOnEntry,
                                             sal_Bool i_bAttachToDependency
                                           )
{
    Sequence< rtl::OUString > aHelpText;
    if( i_rHelpText.getLength() > 0 )
    {
        aHelpText.realloc( 1 );
        *aHelpText.getArray() = i_rHelpText;
    }
    PropertyValue aVal;
    aVal.Name = i_rProperty;
    aVal.Value = makeAny( i_rValue );
    return getUIControlOpt( i_rTitle,
                            aHelpText,
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Edit" ) ),
                            &aVal,
                            i_pDependsOnName,
                            i_nDependsOnEntry,
                            i_bAttachToDependency,
                            0, 0 );
}
