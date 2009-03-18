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
#include "com/sun/star/awt/Size.hpp"
#include "comphelper/processfactory.hxx"

#include <hash_map>
#include <hash_set>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace vcl;

class vcl::ImplPrinterListenerData
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
    MultiSelection                                              maSelection;
    Sequence< PropertyValue >                                   maUIOptions;
    std::vector< PropertyValue >                                maUIProperties;
    std::vector< bool >                                         maUIPropertyEnabled;
    PropertyToIndexMap                                          maPropertyToIndex;
    Link                                                        maOptionChangeHdl;
    ControlDependencyMap                                        maControlDependencies;

    vcl::PrintProgressDialog*                                   mpProgress;

    ImplPrinterListenerData() : mpProgress( NULL ) {}
    ~ImplPrinterListenerData() { delete mpProgress; }
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

    PrintJobAsync( const boost::shared_ptr<PrinterListener>& i_pListener,
                   const JobSetup& i_rInitSetup
                   )
    : mpListener( i_pListener ), maInitSetup( i_rInitSetup )
    {}

    DECL_LINK( ExecJob, void* );
};

IMPL_LINK( PrintJobAsync, ExecJob, void*, EMPTYARG )
{
    Printer::ImplPrintJob( mpListener, maInitSetup );

    // clean up, do not access members after this
    delete this;

    return 0;
}

void Printer::PrintJob( const boost::shared_ptr<PrinterListener>& i_pListener,
                        const JobSetup& i_rInitSetup
                        )
{
    PrintJobAsync* pAsync = new PrintJobAsync( i_pListener, i_rInitSetup );
    Application::PostUserEvent( LINK( pAsync, PrintJobAsync, ExecJob ) );
}

void Printer::ImplPrintJob( const boost::shared_ptr<PrinterListener>& i_pListener,
                            const JobSetup& i_rInitSetup
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
            pListener->getPrinter()->SetCopyCount( static_cast<USHORT>(aDlg.getCopyCount()), aDlg.isCollate() );
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
            i_pListener->createProgressDialog();
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

const MultiSelection& PrinterListener::getPageSelection() const
{
    return mpImplData->maSelection;
}

void PrinterListener::setPrinter( const boost::shared_ptr<Printer>& i_rPrinter )
{
    mpImplData->mpPrinter = i_rPrinter;
}

void PrinterListener::setPageSelection( const MultiSelection& i_rSel )
{
    mpImplData->maSelection = i_rSel;
}

static void modifyJobSetup( Printer* pPrinter, const Sequence< PropertyValue >& i_rProps )
{
    for( sal_Int32 nProperty = 0, nPropertyCount = i_rProps.getLength(); nProperty < nPropertyCount; ++nProperty )
    {
        if( i_rProps[ nProperty ].Name.equalsAscii( "PageSize" ) )
        {
            Size aPageSize;
            awt::Size aSize;
            i_rProps[ nProperty].Value >>= aSize;
            aPageSize.Width() = aSize.Width;
            aPageSize.Height() = aSize.Height;

            Size aCurSize( pPrinter->GetPaperSize() );
            if( aPageSize != aCurSize )
                pPrinter->SetPaperSizeUser( aPageSize );
        }
    }
}

void PrinterListener::printFilteredPage( int i_nPage )
{
    // update progress if necessary
    if( mpImplData->mpProgress )
    {
        mpImplData->mpProgress->tick();
        Application::Reschedule( true );
    }

    // get page parameters
    Sequence< PropertyValue > aPageParm( getPageParameters( i_nPage ) );
    const MapMode aMapMode( MAP_100TH_MM );

    mpImplData->mpPrinter->Push();
    mpImplData->mpPrinter->SetMapMode( aMapMode );

    // modify job setup if necessary
    modifyJobSetup( mpImplData->mpPrinter.get(), aPageParm );

    mpImplData->mpPrinter->EnableOutput( FALSE );

    GDIMetaFile aMtf;
    aMtf.Record( mpImplData->mpPrinter.get() );

    printPage( i_nPage );

    aMtf.Stop();
    aMtf.WindStart();
    mpImplData->mpPrinter->Pop();

    // FIXME: do transparency filtering here when vcl92 is integrated

    mpImplData->mpPrinter->EnableOutput( TRUE );

    // actually print the page
    mpImplData->mpPrinter->StartPage();

    mpImplData->mpPrinter->Push();
    mpImplData->mpPrinter->SetMapMode( MAP_100TH_MM );
    aMtf.WindStart();
    aMtf.Play( mpImplData->mpPrinter.get() );
    mpImplData->mpPrinter->Pop();

    mpImplData->mpPrinter->EndPage();
}

void PrinterListener::jobFinished()
{
}

Sequence< PropertyValue > PrinterListener::getJobProperties( const Sequence< PropertyValue >& i_rMergeList ) const
{
    std::hash_set< rtl::OUString, rtl::OUStringHash > aMergeSet;
    size_t nResultLen = size_t(i_rMergeList.getLength()) + mpImplData->maUIProperties.size();
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
    aResult.realloc( nCur );
    return aResult;
}

const Sequence< beans::PropertyValue >& PrinterListener::getUIOptions() const
{
    return mpImplData->maUIOptions;
}

com::sun::star::beans::PropertyValue* PrinterListener::getValue( const rtl::OUString& i_rProperty )
{
    std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    return it != mpImplData->maPropertyToIndex.end() ? &mpImplData->maUIProperties[it->second] : NULL;
}

const com::sun::star::beans::PropertyValue* PrinterListener::getValue( const rtl::OUString& i_rProperty ) const
{
    std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    return it != mpImplData->maPropertyToIndex.end() ? &mpImplData->maUIProperties[it->second] : NULL;
}

void PrinterListener::setUIOptions( const Sequence< beans::PropertyValue >& i_rOptions )
{
    DBG_ASSERT( mpImplData->maUIOptions.getLength() == 0, "setUIOptions called twice !" );

    mpImplData->maUIOptions = i_rOptions;
    mpImplData->maUIProperties.clear();
    mpImplData->maPropertyToIndex.clear();
    mpImplData->maUIPropertyEnabled.clear();

    for( int i = 0; i < i_rOptions.getLength(); i++ )
    {
        Sequence< beans::PropertyValue > aOptProp;
        i_rOptions[i].Value >>= aOptProp;
        bool bIsEnabled = true;
        bool bHaveProperty = false;
        vcl::ImplPrinterListenerData::ControlDependency aDep;
        for( int n = 0; n < aOptProp.getLength(); n++ )
        {
            const beans::PropertyValue& rEntry( aOptProp[ n ] );
            if( rEntry.Name.equalsAscii( "Property" ) )
            {
                PropertyValue aVal;
                rEntry.Value >>= aVal;
                DBG_ASSERT( mpImplData->maPropertyToIndex.find( aVal.Name )
                            == mpImplData->maPropertyToIndex.end(), "duplicate property entry" );
                mpImplData->maPropertyToIndex[ aVal.Name ] = mpImplData->maUIProperties.size();
                mpImplData->maUIProperties.push_back( aVal );
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
            mpImplData->maUIPropertyEnabled.push_back( bIsEnabled );
            if( aDep.maDependsOnName.getLength() > 0 )
                mpImplData->maControlDependencies[ mpImplData->maUIProperties.back().Name ] = aDep;
        }
    }
}

void PrinterListener::enableUIOption( const rtl::OUString& i_rProperty, bool i_bEnable )
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

bool PrinterListener::isUIOptionEnabled( const rtl::OUString& i_rProperty ) const
{
    bool bEnabled = false;
    std::hash_map< rtl::OUString, size_t, rtl::OUStringHash >::const_iterator it =
        mpImplData->maPropertyToIndex.find( i_rProperty );
    if( it != mpImplData->maPropertyToIndex.end() )
    {
        bEnabled = mpImplData->maUIPropertyEnabled[it->second];

        if( bEnabled )
        {
            // check control dependencies
            vcl::ImplPrinterListenerData::ControlDependencyMap::const_iterator it =
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
                        sal_Int32 nDepVal;
                        sal_Bool bDepVal;
                        if( pVal->Value >>= nDepVal )
                        {
                            bEnabled = (nDepVal == it->second.mnDependsOnEntry);
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

void PrinterListener::setOptionChangeHdl( const Link& i_rHdl )
{
    mpImplData->maOptionChangeHdl = i_rHdl;
}

void PrinterListener::createProgressDialog()
{
    if( ! mpImplData->mpProgress )
    {
        mpImplData->mpProgress = new PrintProgressDialog( NULL, mpImplData->maSelection.GetSelectCount() );
        mpImplData->mpProgress->Show();
    }
}
