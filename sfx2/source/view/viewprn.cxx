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


#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/view/PrintableState.hpp>
#include "com/sun/star/view/XRenderable.hpp"

#include <svl/itempool.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/prnsetup.hxx>
#include <svl/flagitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/printwarningoptions.hxx>
#include <tools/datetime.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewsh.hxx>
#include "viewimp.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/prnmon.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#include "sfxtypes.hxx"
#include <sfx2/event.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>

#include "toolkit/awt/vclxdevice.hxx"

#include "view.hrc"
#include "helpid.hrc"

using namespace com::sun::star;
using namespace com::sun::star::uno;

TYPEINIT1(SfxPrintingHint, SfxHint);

// -----------------------------------------------------------------------
class SfxPrinterController : public vcl::PrinterController, public SfxListener
{
    Any                                     maCompleteSelection;
    Any                                     maSelection;
    Reference< view::XRenderable >          mxRenderable;
    mutable Printer*                        mpLastPrinter;
    mutable Reference<awt::XDevice>         mxDevice;
    SfxViewShell*                           mpViewShell;
    SfxObjectShell*                         mpObjectShell;
    sal_Bool        m_bOrigStatus;
    sal_Bool        m_bNeedsChange;
    sal_Bool        m_bApi;
    sal_Bool        m_bTempPrinter;
    util::DateTime  m_aLastPrinted;
    ::rtl::OUString m_aLastPrintedBy;

    Sequence< beans::PropertyValue > getMergedOptions() const;
    const Any& getSelectionObject() const;
public:
    SfxPrinterController( const boost::shared_ptr<Printer>& i_rPrinter,
                          const Any& i_rComplete,
                          const Any& i_rSelection,
                          const Any& i_rViewProp,
                          const Reference< view::XRenderable >& i_xRender,
                          sal_Bool i_bApi, sal_Bool i_bDirect,
                          SfxViewShell* pView,
                          const uno::Sequence< beans::PropertyValue >& rProps
                        );

    virtual ~SfxPrinterController();
    virtual void Notify( SfxBroadcaster&, const SfxHint& );

    virtual int  getPageCount() const;
    virtual Sequence< beans::PropertyValue > getPageParameters( int i_nPage ) const;
    virtual void printPage( int i_nPage ) const;
    virtual void jobStarted();
    virtual void jobFinished( com::sun::star::view::PrintableState );
};

SfxPrinterController::SfxPrinterController( const boost::shared_ptr<Printer>& i_rPrinter,
                                            const Any& i_rComplete,
                                            const Any& i_rSelection,
                                            const Any& i_rViewProp,
                                            const Reference< view::XRenderable >& i_xRender,
                                            sal_Bool i_bApi, sal_Bool i_bDirect,
                                            SfxViewShell* pView,
                                            const uno::Sequence< beans::PropertyValue >& rProps
                                          )
    : PrinterController( i_rPrinter)
    , maCompleteSelection( i_rComplete )
    , maSelection( i_rSelection )
    , mxRenderable( i_xRender )
    , mpLastPrinter( NULL )
    , mpViewShell( pView )
    , mpObjectShell(0)
    , m_bOrigStatus( sal_False )
    , m_bNeedsChange( sal_False )
    , m_bApi(i_bApi)
    , m_bTempPrinter( i_rPrinter.get() != NULL )
{
    if ( mpViewShell )
    {
        StartListening( *mpViewShell );
        mpObjectShell = mpViewShell->GetObjectShell();
        StartListening( *mpObjectShell );
    }

    // initialize extra ui options
    if( mxRenderable.is() )
    {
        for (sal_Int32 nProp=0; nProp < rProps.getLength(); ++nProp)
            setValue( rProps[nProp].Name, rProps[nProp].Value );

        Sequence< beans::PropertyValue > aRenderOptions( 3 );
        aRenderOptions[0].Name = rtl::OUString( "ExtraPrintUIOptions"  );
        aRenderOptions[1].Name = rtl::OUString( "View"  );
        aRenderOptions[1].Value = i_rViewProp;
        aRenderOptions[2].Name = rtl::OUString( "IsPrinter"  );
        aRenderOptions[2].Value <<= sal_True;
        try
        {
            Sequence< beans::PropertyValue > aRenderParms( mxRenderable->getRenderer( 0 , getSelectionObject(), aRenderOptions ) );
            int nProps = aRenderParms.getLength();
            for( int i = 0; i < nProps; i++ )
            {
                if ( aRenderParms[i].Name == "ExtraPrintUIOptions" )
                {
                    Sequence< beans::PropertyValue > aUIProps;
                    aRenderParms[i].Value >>= aUIProps;
                    setUIOptions( aUIProps );
                    break;
                }
            }
        }
        catch( lang::IllegalArgumentException& )
        {
            // the first renderer should always be available for the UI options,
            // but catch the exception to be safe
        }
    }

    // set some job parameters
    setValue( rtl::OUString( "IsApi"  ), makeAny( i_bApi ) );
    setValue( rtl::OUString( "IsDirect"  ), makeAny( i_bDirect ) );
    setValue( rtl::OUString( "IsPrinter"  ), makeAny( sal_True ) );
    setValue( rtl::OUString( "View"  ), i_rViewProp );
}

void SfxPrinterController::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        if ( ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        {
            EndListening(*mpViewShell);
            EndListening(*mpObjectShell);
            mpViewShell = 0;
            mpObjectShell = 0;
        }
    }
}

SfxPrinterController::~SfxPrinterController()
{
}

const Any& SfxPrinterController::getSelectionObject() const
{
    const beans::PropertyValue* pVal = getValue( rtl::OUString( "PrintSelectionOnly"  ) );
    if( pVal )
    {
        sal_Bool bSel = sal_False;
        pVal->Value >>= bSel;
        return bSel ? maSelection : maCompleteSelection;
    }

    sal_Int32 nChoice = 0;
    pVal = getValue( rtl::OUString( "PrintContent"  ) );
    if( pVal )
        pVal->Value >>= nChoice;
    return (nChoice > 1) ? maSelection : maCompleteSelection;
}

Sequence< beans::PropertyValue > SfxPrinterController::getMergedOptions() const
{
    boost::shared_ptr<Printer> pPrinter( getPrinter() );
    if( pPrinter.get() != mpLastPrinter )
    {
        mpLastPrinter = pPrinter.get();
        VCLXDevice* pXDevice = new VCLXDevice();
        pXDevice->SetOutputDevice( mpLastPrinter );
        mxDevice = Reference< awt::XDevice >( pXDevice );
    }

    Sequence< beans::PropertyValue > aRenderOptions( 1 );
    aRenderOptions[ 0 ].Name = rtl::OUString( "RenderDevice"  );
    aRenderOptions[ 0 ].Value <<= mxDevice;

    aRenderOptions = getJobProperties( aRenderOptions );
    return aRenderOptions;
}

int SfxPrinterController::getPageCount() const
{
    int nPages = 0;
    boost::shared_ptr<Printer> pPrinter( getPrinter() );
    if( mxRenderable.is() && pPrinter )
    {
        Sequence< beans::PropertyValue > aJobOptions( getMergedOptions() );
        try
        {
            nPages = mxRenderable->getRendererCount( getSelectionObject(), aJobOptions );
        }
        catch (lang::DisposedException &)
        {
            OSL_TRACE("SfxPrinterController: document disposed while printing");
            const_cast<SfxPrinterController*>(this)->setJobState(
                    view::PrintableState_JOB_ABORTED);
        }
    }
    return nPages;
}

Sequence< beans::PropertyValue > SfxPrinterController::getPageParameters( int i_nPage ) const
{
    boost::shared_ptr<Printer> pPrinter( getPrinter() );
    Sequence< beans::PropertyValue > aResult;

    if( mxRenderable.is() && pPrinter )
    {
        Sequence< beans::PropertyValue > aJobOptions( getMergedOptions() );
        try
        {
            aResult = mxRenderable->getRenderer( i_nPage, getSelectionObject(), aJobOptions );
        }
        catch( lang::IllegalArgumentException& )
        {
        }
        catch (lang::DisposedException &)
        {
            OSL_TRACE("SfxPrinterController: document disposed while printing");
            const_cast<SfxPrinterController*>(this)->setJobState(
                    view::PrintableState_JOB_ABORTED);
        }
    }
    return aResult;
}

void SfxPrinterController::printPage( int i_nPage ) const
{
    boost::shared_ptr<Printer> pPrinter( getPrinter() );
    if( mxRenderable.is() && pPrinter )
    {
        Sequence< beans::PropertyValue > aJobOptions( getMergedOptions() );
        try
        {
            mxRenderable->render( i_nPage, getSelectionObject(), aJobOptions );
        }
        catch( lang::IllegalArgumentException& )
        {
            // don't care enough about nonexistant page here
            // to provoke a crash
        }
        catch (lang::DisposedException &)
        {
            OSL_TRACE("SfxPrinterController: document disposed while printing");
            const_cast<SfxPrinterController*>(this)->setJobState(
                    view::PrintableState_JOB_ABORTED);
        }
    }
}

void SfxPrinterController::jobStarted()
{
    if ( mpObjectShell )
    {
        m_bOrigStatus = mpObjectShell->IsEnableSetModified();

        // check configuration: shall update of printing information in DocInfo set the document to "modified"?
        if ( m_bOrigStatus && !SvtPrintWarningOptions().IsModifyDocumentOnPrintingAllowed() )
        {
            mpObjectShell->EnableSetModified( sal_False );
            m_bNeedsChange = sal_True;
        }

        // refresh document info
        uno::Reference<document::XDocumentProperties> xDocProps(mpObjectShell->getDocProperties());
        m_aLastPrintedBy = xDocProps->getPrintedBy();
        m_aLastPrinted = xDocProps->getPrintDate();

        xDocProps->setPrintedBy( mpObjectShell->IsUseUserData()
            ? ::rtl::OUString( SvtUserOptions().GetFullName() )
            : ::rtl::OUString() );
        ::DateTime now( ::DateTime::SYSTEM );

        xDocProps->setPrintDate( util::DateTime(
            now.Get100Sec(), now.GetSec(), now.GetMin(), now.GetHour(),
            now.GetDay(), now.GetMonth(), now.GetYear() ) );

        // FIXME: how to get all print options incl. AdditionalOptions easily?
        uno::Sequence < beans::PropertyValue > aOpts;
        mpObjectShell->Broadcast( SfxPrintingHint( view::PrintableState_JOB_STARTED, aOpts ) );
    }
}

void SfxPrinterController::jobFinished( com::sun::star::view::PrintableState nState )
{
    if ( mpObjectShell )
    {
        bool bCopyJobSetup = false;
        mpObjectShell->Broadcast( SfxPrintingHint( nState ) );
        switch ( nState )
        {
            case view::PrintableState_JOB_FAILED :
            {
                // "real" problem (not simply printing cancelled by user)
                rtl::OUString aMsg( SfxResId(STR_NOSTARTPRINTER).toString() );
                if ( !m_bApi )
                    ErrorBox( mpViewShell->GetWindow(), WB_OK | WB_DEF_OK,  aMsg ).Execute();
                // intentionally no break
            }
            case view::PrintableState_JOB_ABORTED :
            {
                // printing not successful, reset DocInfo
                uno::Reference<document::XDocumentProperties> xDocProps(mpObjectShell->getDocProperties());
                xDocProps->setPrintedBy(m_aLastPrintedBy);
                xDocProps->setPrintDate(m_aLastPrinted);
                break;
            }

            case view::PrintableState_JOB_SPOOLED :
            case view::PrintableState_JOB_COMPLETED :
            {
                SfxBindings& rBind = mpViewShell->GetViewFrame()->GetBindings();
                rBind.Invalidate( SID_PRINTDOC );
                rBind.Invalidate( SID_PRINTDOCDIRECT );
                rBind.Invalidate( SID_SETUPPRINTER );
                bCopyJobSetup = ! m_bTempPrinter;
                break;
            }

            default:
                break;
        }

        if( bCopyJobSetup && mpViewShell )
        {
            // #i114306#
            // Note: this possibly creates a printer that gets immediately replaced
            // by a new one. The reason for this is that otherwise we would not get
            // the printer's SfxItemSet here to copy. Awkward, but at the moment there is no
            // other way here to get the item set.
            SfxPrinter* pDocPrt = mpViewShell->GetPrinter(sal_True);
            if( pDocPrt )
            {
                if( pDocPrt->GetName() == getPrinter()->GetName() )
                    pDocPrt->SetJobSetup( getPrinter()->GetJobSetup() );
                else
                {
                    SfxPrinter* pNewPrt = new SfxPrinter( pDocPrt->GetOptions().Clone(), getPrinter()->GetName() );
                    pNewPrt->SetJobSetup( getPrinter()->GetJobSetup() );
                    mpViewShell->SetPrinter( pNewPrt, SFX_PRINTER_PRINTER | SFX_PRINTER_JOBSETUP );
                }
            }
        }

        if ( m_bNeedsChange )
            mpObjectShell->EnableSetModified( m_bOrigStatus );

        if ( mpViewShell )
        {
            mpViewShell->pImp->m_pPrinterController.reset();
        }
    }
}

//====================================================================

class SfxDialogExecutor_Impl

/*  [Description]

    An instance of this class is created for the life span of the
    printer dialogue, to create in its click handler for the additions by the
    virtual method of the derived SfxViewShell generated print options dialogue
    and to cache the options set there as SfxItemSet.
*/

{
private:
    SfxViewShell*           _pViewSh;
    PrinterSetupDialog*     _pSetupParent;
    SfxItemSet*             _pOptions;
    sal_Bool                _bHelpDisabled;

    DECL_LINK( Execute, void * );

public:
            SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrinterSetupDialog* pParent );
            ~SfxDialogExecutor_Impl() { delete _pOptions; }

    Link                GetLink() const { return LINK( this, SfxDialogExecutor_Impl, Execute); }
    const SfxItemSet*   GetOptions() const { return _pOptions; }
    void                DisableHelp() { _bHelpDisabled = sal_True; }
};

//--------------------------------------------------------------------

SfxDialogExecutor_Impl::SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrinterSetupDialog* pParent ) :

    _pViewSh        ( pViewSh ),
    _pSetupParent   ( pParent ),
    _pOptions       ( NULL ),
    _bHelpDisabled  ( sal_False )

{
}

//--------------------------------------------------------------------

IMPL_LINK_NOARG(SfxDialogExecutor_Impl, Execute)
{
    // Options noted locally
    if ( !_pOptions )
    {
        DBG_ASSERT( _pSetupParent, "no dialog parent" );
        if( _pSetupParent )
            _pOptions = ( (SfxPrinter*)_pSetupParent->GetPrinter() )->GetOptions().Clone();
    }

    // Create Dialog
    SfxPrintOptionsDialog* pDlg = new SfxPrintOptionsDialog( static_cast<Window*>(_pSetupParent),
                                                             _pViewSh, _pOptions );
    if ( _bHelpDisabled )
        pDlg->DisableHelp();
    if ( pDlg->Execute() == RET_OK )
    {
        delete _pOptions;
        _pOptions = pDlg->GetOptions().Clone();

    }
    delete pDlg;

    return 0;
}

//-------------------------------------------------------------------------

SfxPrinter* SfxViewShell::SetPrinter_Impl( SfxPrinter *pNewPrinter )


/* Internal method for setting the differences between 'pNewPrinter' to the
   current printer. pNewPrinter is either taken over or deleted.
*/

{
    // get current Printer
    SfxPrinter *pDocPrinter = GetPrinter();

    // Evaluate Printer Options
    bool bOriToDoc = false;
    bool bSizeToDoc = false;
    if ( &pDocPrinter->GetOptions() )
    {
        sal_uInt16 nWhich = GetPool().GetWhich(SID_PRINTER_CHANGESTODOC);
        const SfxFlagItem *pFlagItem = 0;
        pDocPrinter->GetOptions().GetItemState( nWhich, sal_False, (const SfxPoolItem**) &pFlagItem );
        bOriToDoc = pFlagItem ? (pFlagItem->GetValue() & SFX_PRINTER_CHG_ORIENTATION) : sal_False;
        bSizeToDoc = pFlagItem ? (pFlagItem->GetValue() & SFX_PRINTER_CHG_SIZE) : sal_False;
    }

    // Determine the previous format and size
    Orientation eOldOri = pDocPrinter->GetOrientation();
    Size aOldPgSz = pDocPrinter->GetPaperSizePixel();

    // Determine the new format and size
    Orientation eNewOri = pNewPrinter->GetOrientation();
    Size aNewPgSz = pNewPrinter->GetPaperSizePixel();

    // Determine the changes in page format
    sal_Bool bOriChg = (eOldOri != eNewOri) && bOriToDoc;
    sal_Bool bPgSzChg = ( aOldPgSz.Height() !=
            ( bOriChg ? aNewPgSz.Width() : aNewPgSz.Height() ) ||
            aOldPgSz.Width() !=
            ( bOriChg ? aNewPgSz.Height() : aNewPgSz.Width() ) ) &&
            bSizeToDoc;

    // Message and Flags for page format, summaries changes
    rtl::OUString aMsg;
    sal_uInt16 nNewOpt=0;
    if( bOriChg && bPgSzChg )
    {
        aMsg = SfxResId(STR_PRINT_NEWORISIZE).toString();
        nNewOpt = SFX_PRINTER_CHG_ORIENTATION | SFX_PRINTER_CHG_SIZE;
    }
    else if (bOriChg )
    {
        aMsg = SfxResId(STR_PRINT_NEWORI).toString();
        nNewOpt = SFX_PRINTER_CHG_ORIENTATION;
    }
    else if (bPgSzChg)
    {
        aMsg = SfxResId(STR_PRINT_NEWSIZE).toString();
        nNewOpt = SFX_PRINTER_CHG_SIZE;
    }

    // Summaries in this variable what has been changed.
    sal_uInt16 nChangedFlags = 0;

    // Ask if possible, if page format should be taken over from printer.
    if ( ( bOriChg  || bPgSzChg ) &&
        RET_YES == QueryBox(0, WB_YES_NO | WB_DEF_OK, aMsg).Execute() )
    // Flags wich changes for  <SetPrinter(SfxPrinter*)> are maintained
    nChangedFlags |= nNewOpt;

    // For the MAC to have its "temporary of class String" in next if()
    String aTempPrtName = pNewPrinter->GetName();
    String aDocPrtName = pDocPrinter->GetName();

    // Was the printer selection changed from Default to Specific
    // or the other way around?
    if ( (aTempPrtName != aDocPrtName) || (pDocPrinter->IsDefPrinter() != pNewPrinter->IsDefPrinter()) )
    {
        nChangedFlags |= SFX_PRINTER_PRINTER|SFX_PRINTER_JOBSETUP;
        pDocPrinter = pNewPrinter;
    }
    else
    {
        // Compare extra options
        if ( ! (pNewPrinter->GetOptions() == pDocPrinter->GetOptions()) )
        {
            // Option have changed
            pDocPrinter->SetOptions( pNewPrinter->GetOptions() );
            nChangedFlags |= SFX_PRINTER_OPTIONS;
        }

        // Compare JobSetups
        JobSetup aNewJobSetup = pNewPrinter->GetJobSetup();
        JobSetup aOldJobSetup = pDocPrinter->GetJobSetup();
        if ( aNewJobSetup != aOldJobSetup )
        {
            nChangedFlags |= SFX_PRINTER_JOBSETUP;
        }

        // Keep old changed Printer.
        pDocPrinter->SetPrinterProps( pNewPrinter );
        delete pNewPrinter;
    }

    if ( 0 != nChangedFlags )
        // SetPrinter will delete the old printer if it changes
        SetPrinter( pDocPrinter, nChangedFlags );
    return pDocPrinter;
}

//-------------------------------------------------------------------------
// Sadly enough the problem arises with WIN32 that nothing is printed when
// SID_PRINTDOCDIRECT auflaueft. At the moment the only known solution in this
// case is to turn off the optimazation.
#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

void SfxViewShell::ExecPrint( const uno::Sequence < beans::PropertyValue >& rProps, sal_Bool bIsAPI, sal_Bool bIsDirect )
{
    // get the current selection; our controller should know it
    Reference< frame::XController > xController( GetController() );
    Reference< view::XSelectionSupplier > xSupplier( xController, UNO_QUERY );

    Any aSelection;
    if( xSupplier.is() )
        aSelection = xSupplier->getSelection();
    else
        aSelection <<= GetObjectShell()->GetModel();
    Any aComplete( makeAny( GetObjectShell()->GetModel() ) );
    Any aViewProp( makeAny( xController ) );
    boost::shared_ptr<Printer> aPrt;

    const beans::PropertyValue* pVal = rProps.getConstArray();
    for( sal_Int32 i = 0; i < rProps.getLength(); i++ )
    {
        if ( pVal[i].Name == "PrinterName" )
        {
            rtl::OUString aPrinterName;
            pVal[i].Value >>= aPrinterName;
            aPrt.reset( new Printer( aPrinterName ) );
            break;
        }
    }

    boost::shared_ptr<vcl::PrinterController> pController( new SfxPrinterController(
                                                                               aPrt,
                                                                               aComplete,
                                                                               aSelection,
                                                                               aViewProp,
                                                                               GetRenderable(),
                                                                               bIsAPI,
                                                                               bIsDirect,
                                                                               this,
                                                                               rProps
                                                                               ) );
    pImp->m_pPrinterController = pController;

    SfxObjectShell *pObjShell = GetObjectShell();
    pController->setValue( rtl::OUString( "JobName"  ),
                        makeAny( rtl::OUString( pObjShell->GetTitle(0) ) ) );

    // FIXME: job setup
    SfxPrinter* pDocPrt = GetPrinter(sal_False);
    JobSetup aJobSetup = pDocPrt ? pDocPrt->GetJobSetup() : GetJobSetup();
    if( bIsDirect )
        aJobSetup.SetValue( String( "IsQuickJob"  ),
                            String( "true"  ) );

    Printer::PrintJob( pController, aJobSetup );
}

Printer* SfxViewShell::GetActivePrinter() const
{
    return (pImp->m_pPrinterController)
        ?  pImp->m_pPrinterController->getPrinter().get() : 0;
}

void SfxViewShell::ExecPrint_Impl( SfxRequest &rReq )
{
    sal_uInt16              nDialogRet = RET_CANCEL;
    SfxPrinter*             pPrinter = 0;
    SfxDialogExecutor_Impl* pExecutor = 0;
    bool                    bSilent = false;

    // does the function have been called by the user interface or by an API call
    sal_Bool bIsAPI = rReq.IsAPI();
    if ( bIsAPI )
    {
        // the function have been called by the API

        // Should it be visible on the user interface,
        // should it launch popup dialogue ?
        SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem, SID_SILENT, sal_False);
        bSilent = pSilentItem && pSilentItem->GetValue();
    }

    // no help button in dialogs if called from the help window
    // (pressing help button would exchange the current page inside the help
    // document that is going to be printed!)
    String aHelpFilterName( DEFINE_CONST_UNICODE("writer_web_HTML_help") );
    SfxMedium* pMedium = GetViewFrame()->GetObjectShell()->GetMedium();
    const SfxFilter* pFilter = pMedium ? pMedium->GetFilter() : NULL;
    sal_Bool bPrintOnHelp = ( pFilter && pFilter->GetFilterName() == aHelpFilterName );

    const sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_PRINTDOC: // display the printer selection and properties dialogue : File > Print…
        case SID_PRINTDOCDIRECT: // Print the document directly, without displaying the dialogue
        {
            SfxObjectShell* pDoc = GetObjectShell();

            // derived class may decide to abort this
            if( pDoc == NULL || !pDoc->QuerySlotExecutable( nId ) )
            {
                rReq.SetReturnValue( SfxBoolItem( 0, sal_False ) );
                return;
            }

            if ( !bSilent && pDoc->QueryHiddenInformation( WhenPrinting, NULL ) != RET_YES )
                return;

            uno::Sequence < beans::PropertyValue > aProps;
            if ( bIsAPI )
            {
                // supported properties:
                // String PrinterName
                // String FileName
                // Int16 From
                // Int16 To
                // In16 Copies
                // String RangeText
                // bool Selection
                // bool Asynchron
                // bool Collate
                // bool Silent

                // the TransformItems function overwrite aProps
                TransformItems( nId, *rReq.GetArgs(), aProps, GetInterface()->GetSlot(nId) );

                for ( sal_Int32 nProp=0; nProp < aProps.getLength(); ++nProp )
                {
                    if ( aProps[nProp].Name == "Copies" )
                    {
                        aProps[nProp]. Name = rtl::OUString("CopyCount");
                    }
                    else if ( aProps[nProp].Name == "RangeText" )
                    {
                        aProps[nProp]. Name = rtl::OUString("Pages");
                    }
                    else if ( aProps[nProp].Name == "Asynchron" )
                    {
                        aProps[nProp]. Name = rtl::OUString("Wait");
                        sal_Bool bAsynchron = sal_False;
                        aProps[nProp].Value >>= bAsynchron;
                        aProps[nProp].Value <<= (sal_Bool) (!bAsynchron);
                    }
                    else if ( aProps[nProp].Name == "Silent" )
                    {
                        aProps[nProp]. Name = rtl::OUString("MonitorVisible");
                        sal_Bool bPrintSilent = sal_False;
                        aProps[nProp].Value >>= bPrintSilent;
                        aProps[nProp].Value <<= (sal_Bool) (!bPrintSilent);
                    }
                }
            }

            // we will add the "PrintSelectionOnly" or "HideHelpButton" properties
            // we have to increase the capacity of aProps
            sal_Int32 nLen = aProps.getLength();
            aProps.realloc( nLen + 1 );

            // HACK: writer sets the SID_SELECTION item when printing directly and expects
            // to get only the selection document in that case (see getSelectionObject)
            // however it also reacts to the PrintContent property. We need this distinction here, too,
            // else one of the combinations print / print direct and selection / all will not work.
            // it would be better if writer handled this internally
            if( nId == SID_PRINTDOCDIRECT )
            {
                // should we print only the selection or the whole document
                SFX_REQUEST_ARG(rReq, pSelectItem, SfxBoolItem, SID_SELECTION, sal_False);
                sal_Bool bSelection = ( pSelectItem != NULL && pSelectItem->GetValue() );

                aProps[nLen].Name = rtl::OUString( "PrintSelectionOnly"  );
                aProps[nLen].Value = makeAny( bSelection );
            }
            else // if nId == SID_PRINTDOC ; nothing to do with the previous HACK
            {
                // should the printer selection and properties dialogue display an help button
                aProps[nLen].Name = rtl::OUString( "HideHelpButton" );
                aProps[nLen].Value = makeAny( bPrintOnHelp );
            }

            ExecPrint( aProps, bIsAPI, (nId == SID_PRINTDOCDIRECT) );

            // FIXME: Recording
            rReq.Done();
            break;
        }
        case SID_SETUPPRINTER : // display the printer settings dialogue : File > Printer Settings…
        case SID_PRINTER_NAME : // only for recorded macros
        {
            // get printer and printer settings from the document
            SfxPrinter *pDocPrinter = GetPrinter(sal_True);

            // look for printer in parameters
            SFX_REQUEST_ARG( rReq, pPrinterItem, SfxStringItem, SID_PRINTER_NAME, sal_False );
            if ( pPrinterItem )
            {
                // use PrinterName parameter to create a printer
                pPrinter = new SfxPrinter( pDocPrinter->GetOptions().Clone(), ((const SfxStringItem*) pPrinterItem)->GetValue() );

                // if printer is unknown, it can't be used - now printer from document will be used
                if ( !pPrinter->IsKnown() )
                    DELETEZ(pPrinter);
            }

            if ( SID_PRINTER_NAME == nId )
            {
                // just set a recorded printer name
                if ( pPrinter )
                    SetPrinter( pPrinter, SFX_PRINTER_PRINTER  );
                return;
            }

            // no PrinterName parameter in ItemSet or the PrinterName points to an unknown printer
            if ( !pPrinter )
                // use default printer from document
                pPrinter = pDocPrinter;

            if( !pPrinter || !pPrinter->IsValid() )
            {
                // no valid printer either in ItemSet or at the document
                if ( !bSilent )
                    ErrorBox( NULL, WB_OK | WB_DEF_OK, SfxResId(STR_NODEFPRINTER).toString() ).Execute();

                rReq.SetReturnValue(SfxBoolItem(0,sal_False));

                break;
            }

            // FIXME: printer isn't used for printing anymore!
            if( pPrinter->IsPrinting() )
            {
                // if printer is busy, abort configuration
                if ( !bSilent )
                    InfoBox( NULL, SfxResId(STR_ERROR_PRINTER_BUSY).toString() ).Execute();
                rReq.SetReturnValue(SfxBoolItem(0,sal_False));

                return;
            }

            // if no arguments are given, retrieve them from a dialog
            if ( !bIsAPI )
            {
                // PrinterDialog needs a temporary printer
                SfxPrinter* pDlgPrinter = pPrinter->Clone();
                nDialogRet = 0;

                // execute PrinterSetupDialog
                PrinterSetupDialog* pPrintSetupDlg = new PrinterSetupDialog( GetWindow() );

                if (pImp->m_bHasPrintOptions)
                {
                    // additional controls for dialog
                    pExecutor = new SfxDialogExecutor_Impl( this, pPrintSetupDlg );
                    if ( bPrintOnHelp )
                        pExecutor->DisableHelp();
                    pPrintSetupDlg->SetOptionsHdl( pExecutor->GetLink() );
                }

                pPrintSetupDlg->SetPrinter( pDlgPrinter );
                nDialogRet = pPrintSetupDlg->Execute();

                if ( pExecutor && pExecutor->GetOptions() )
                {
                    if ( nDialogRet == RET_OK )
                        // remark: have to be recorded if possible!
                        pDlgPrinter->SetOptions( *pExecutor->GetOptions() );
                    else
                    {
                        pPrinter->SetOptions( *pExecutor->GetOptions() );
                        SetPrinter( pPrinter, SFX_PRINTER_OPTIONS );
                    }
                }

                DELETEZ( pPrintSetupDlg );

                // no recording of PrinterSetup except printer name (is printer dependent)
                rReq.Ignore();

                if ( nDialogRet == RET_OK )
                {
                    if ( pPrinter->GetName() != pDlgPrinter->GetName() )
                    {
                        // user has changed the printer -> macro recording
                        SfxRequest aReq( GetViewFrame(), SID_PRINTER_NAME );
                        aReq.AppendItem( SfxStringItem( SID_PRINTER_NAME, pDlgPrinter->GetName() ) );
                        aReq.Done();
                    }
                    if ( nId == SID_SETUPPRINTER )
                    {
                        rReq.AppendItem( SfxBoolItem( SID_DIALOG_RETURN, sal_True ) );
                    }

                    // take the changes made in the dialog
                    pPrinter = SetPrinter_Impl( pDlgPrinter );

                    // forget new printer, it was taken over (as pPrinter) or deleted
                    pDlgPrinter = NULL;

                }
                else
                {
                    // PrinterDialog is used to transfer information on printing,
                    // so it will only be deleted here if dialog was cancelled
                    DELETEZ( pDlgPrinter );
                    rReq.Ignore();
                    if ( SID_PRINTDOC == nId )
                        rReq.SetReturnValue(SfxBoolItem(0,sal_False));
                    if ( nId == SID_SETUPPRINTER )
                        rReq.AppendItem( SfxBoolItem( SID_DIALOG_RETURN, sal_False ) );
                }
            }
        }
    }
}

// Turn on optimazation again.
#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

//--------------------------------------------------------------------

SfxPrinter* SfxViewShell::GetPrinter( sal_Bool /*bCreate*/ )
{
    return 0;
}

//--------------------------------------------------------------------

sal_uInt16 SfxViewShell::SetPrinter( SfxPrinter* /*pNewPrinter*/, sal_uInt16 /*nDiffFlags*/, bool )
{
    return 0;
}

//--------------------------------------------------------------------

SfxTabPage* SfxViewShell::CreatePrintOptionsPage
(
    Window*             /*pParent*/,
    const SfxItemSet&   /*rOptions*/
)
{
    return 0;
}

JobSetup SfxViewShell::GetJobSetup() const
{
    return JobSetup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
