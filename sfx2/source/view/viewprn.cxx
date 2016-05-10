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

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/view/XRenderable.hpp>

#include <svl/itempool.hxx>
#include <vcl/layout.hxx>
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
#include <sfx2/sfxresid.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#include "sfxtypes.hxx"
#include <sfx2/event.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>

#include <toolkit/awt/vclxdevice.hxx>

#include "view.hrc"
#include "helpid.hrc"

using namespace com::sun::star;
using namespace com::sun::star::uno;

class SfxPrinterController : public vcl::PrinterController, public SfxListener
{
    Any                                     maCompleteSelection;
    Any                                     maSelection;
    Reference< view::XRenderable >          mxRenderable;
    mutable VclPtr<Printer>                 mpLastPrinter;
    mutable Reference<awt::XDevice>         mxDevice;
    SfxViewShell*                           mpViewShell;
    SfxObjectShell*                         mpObjectShell;
    bool        m_bOrigStatus;
    bool        m_bNeedsChange;
    bool        m_bApi;
    bool        m_bTempPrinter;
    util::DateTime  m_aLastPrinted;
    OUString m_aLastPrintedBy;

    Sequence< beans::PropertyValue > getMergedOptions() const;
    const Any& getSelectionObject() const;

public:
    SfxPrinterController( const VclPtr<Printer>& i_rPrinter,
                          const Any& i_rComplete,
                          const Any& i_rSelection,
                          const Any& i_rViewProp,
                          const Reference< view::XRenderable >& i_xRender,
                          bool i_bApi, bool i_bDirect,
                          SfxViewShell* pView,
                          const uno::Sequence< beans::PropertyValue >& rProps
                        );

    virtual ~SfxPrinterController();
    virtual void Notify( SfxBroadcaster&, const SfxHint& ) override;

    virtual int  getPageCount() const override;
    virtual Sequence< beans::PropertyValue > getPageParameters( int i_nPage ) const override;
    virtual void printPage( int i_nPage ) const override;
    virtual void jobStarted() override;
    virtual void jobFinished( css::view::PrintableState ) override;
};

SfxPrinterController::SfxPrinterController( const VclPtr<Printer>& i_rPrinter,
                                            const Any& i_rComplete,
                                            const Any& i_rSelection,
                                            const Any& i_rViewProp,
                                            const Reference< view::XRenderable >& i_xRender,
                                            bool i_bApi, bool i_bDirect,
                                            SfxViewShell* pView,
                                            const uno::Sequence< beans::PropertyValue >& rProps
                                          )
    : PrinterController( i_rPrinter)
    , maCompleteSelection( i_rComplete )
    , maSelection( i_rSelection )
    , mxRenderable( i_xRender )
    , mpLastPrinter( nullptr )
    , mpViewShell( pView )
    , mpObjectShell(nullptr)
    , m_bOrigStatus( false )
    , m_bNeedsChange( false )
    , m_bApi(i_bApi)
    , m_bTempPrinter( i_rPrinter.get() != nullptr )
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
        aRenderOptions[0].Name = "ExtraPrintUIOptions";
        aRenderOptions[1].Name = "View" ;
        aRenderOptions[1].Value = i_rViewProp;
        aRenderOptions[2].Name = "IsPrinter";
        aRenderOptions[2].Value <<= true;
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
                }
                else if( aRenderParms[i].Name == "NUp" )
                {
                    setValue( aRenderParms[i].Name, aRenderParms[i].Value );
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
    setValue( "IsApi", makeAny( i_bApi ) );
    setValue( "IsDirect", makeAny( i_bDirect ) );
    setValue( "IsPrinter", makeAny( true ) );
    setValue( "View", i_rViewProp );
}

void SfxPrinterController::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint )
    {
        if ( pSimpleHint->GetId() == SFX_HINT_DYING )
        {
            EndListening(*mpViewShell);
            EndListening(*mpObjectShell);
            mpViewShell = nullptr;
            mpObjectShell = nullptr;
        }
    }
}

SfxPrinterController::~SfxPrinterController()
{
}

const Any& SfxPrinterController::getSelectionObject() const
{
    const beans::PropertyValue* pVal = getValue( OUString( "PrintSelectionOnly"  ) );
    if( pVal )
    {
        bool bSel = false;
        pVal->Value >>= bSel;
        return bSel ? maSelection : maCompleteSelection;
    }

    sal_Int32 nChoice = 0;
    pVal = getValue( OUString( "PrintContent"  ) );
    if( pVal )
        pVal->Value >>= nChoice;
    return (nChoice > 1) ? maSelection : maCompleteSelection;
}

Sequence< beans::PropertyValue > SfxPrinterController::getMergedOptions() const
{
    VclPtr<Printer> xPrinter( getPrinter() );
    if( xPrinter.get() != mpLastPrinter )
    {
        mpLastPrinter = xPrinter.get();
        VCLXDevice* pXDevice = new VCLXDevice();
        pXDevice->SetOutputDevice( mpLastPrinter );
        mxDevice.set( pXDevice );
    }

    Sequence< beans::PropertyValue > aRenderOptions( 1 );
    aRenderOptions[ 0 ].Name = "RenderDevice";
    aRenderOptions[ 0 ].Value <<= mxDevice;

    aRenderOptions = getJobProperties( aRenderOptions );
    return aRenderOptions;
}

int SfxPrinterController::getPageCount() const
{
    int nPages = 0;
    VclPtr<Printer> xPrinter( getPrinter() );
    if( mxRenderable.is() && xPrinter )
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
    VclPtr<Printer> xPrinter( getPrinter() );
    Sequence< beans::PropertyValue > aResult;

    if (mxRenderable.is() && xPrinter)
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
    VclPtr<Printer> xPrinter( getPrinter() );
    if( mxRenderable.is() && xPrinter )
    {
        Sequence< beans::PropertyValue > aJobOptions( getMergedOptions() );
        try
        {
            mxRenderable->render( i_nPage, getSelectionObject(), aJobOptions );
        }
        catch( lang::IllegalArgumentException& )
        {
            // don't care enough about nonexistent page here
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
            mpObjectShell->EnableSetModified( false );
            m_bNeedsChange = true;
        }

        // refresh document info
        uno::Reference<document::XDocumentProperties> xDocProps(mpObjectShell->getDocProperties());
        m_aLastPrintedBy = xDocProps->getPrintedBy();
        m_aLastPrinted = xDocProps->getPrintDate();

        xDocProps->setPrintedBy( mpObjectShell->IsUseUserData()
            ? OUString( SvtUserOptions().GetFullName() )
            : OUString() );
        ::DateTime now( ::DateTime::SYSTEM );

        xDocProps->setPrintDate( now.GetUNODateTime() );

        SfxGetpApp()->NotifyEvent( SfxEventHint(SFX_EVENT_PRINTDOC, GlobalEventConfig::GetEventName( GlobalEventId::PRINTDOC ), mpObjectShell ) );
        uno::Sequence < beans::PropertyValue > aOpts;
        aOpts = getJobProperties( aOpts );

        uno::Reference< frame::XController2 > xController;
        if ( mpViewShell )
            xController.set( mpViewShell->GetController(), uno::UNO_QUERY );

        mpObjectShell->Broadcast( SfxPrintingHint(
            view::PrintableState_JOB_STARTED, aOpts, mpObjectShell, xController ) );
    }
}

void SfxPrinterController::jobFinished( css::view::PrintableState nState )
{
    if ( mpObjectShell )
    {
        bool bCopyJobSetup = false;
        mpObjectShell->Broadcast( SfxPrintingHint( nState ) );
        switch ( nState )
        {
            case view::PrintableState_JOB_SPOOLING_FAILED :
            case view::PrintableState_JOB_FAILED :
            {
                // "real" problem (not simply printing cancelled by user)
                OUString aMsg( SfxResId(STR_NOSTARTPRINTER).toString() );
                if ( !m_bApi )
                    ScopedVclPtrInstance<MessageDialog>::Create(mpViewShell->GetWindow(), aMsg)->Execute();
                SAL_FALLTHROUGH;
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
            SfxPrinter* pDocPrt = mpViewShell->GetPrinter(true);
            if( pDocPrt )
            {
                if( pDocPrt->GetName() == getPrinter()->GetName() )
                    pDocPrt->SetJobSetup( getPrinter()->GetJobSetup() );
                else
                {
                    VclPtr<SfxPrinter> pNewPrt = VclPtr<SfxPrinter>::Create( pDocPrt->GetOptions().Clone(), getPrinter()->GetName() );
                    pNewPrt->SetJobSetup( getPrinter()->GetJobSetup() );
                    mpViewShell->SetPrinter( pNewPrt, SfxPrinterChangeFlags::PRINTER | SfxPrinterChangeFlags::JOBSETUP );
                }
            }
        }

        if ( m_bNeedsChange )
            mpObjectShell->EnableSetModified( m_bOrigStatus );

        if ( mpViewShell )
        {
            mpViewShell->pImp->m_xPrinterController.reset();
        }
    }
}

/**
    An instance of this class is created for the life span of the
    printer dialogue, to create in its click handler for the additions by the
    virtual method of the derived SfxViewShell generated print options dialogue
    and to cache the options set there as SfxItemSet.
*/
class SfxDialogExecutor_Impl
{
private:
    SfxViewShell*           _pViewSh;
    VclPtr<PrinterSetupDialog>  _pSetupParent;
    SfxItemSet*             _pOptions;
    bool                    _bHelpDisabled;

    DECL_LINK_TYPED( Execute, Button*, void );

public:
            SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrinterSetupDialog* pParent );
            ~SfxDialogExecutor_Impl() { delete _pOptions; }

    Link<Button*, void> GetLink() const { return LINK(const_cast<SfxDialogExecutor_Impl*>(this), SfxDialogExecutor_Impl, Execute); }
    const SfxItemSet*   GetOptions() const { return _pOptions; }
    void                DisableHelp() { _bHelpDisabled = true; }
};

SfxDialogExecutor_Impl::SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrinterSetupDialog* pParent ) :

    _pViewSh        ( pViewSh ),
    _pSetupParent   ( pParent ),
    _pOptions       ( nullptr ),
    _bHelpDisabled  ( false )

{
}

IMPL_LINK_NOARG_TYPED(SfxDialogExecutor_Impl, Execute, Button*, void)
{
    // Options noted locally
    if ( !_pOptions )
    {
        DBG_ASSERT( _pSetupParent, "no dialog parent" );
        if( _pSetupParent )
            _pOptions = static_cast<SfxPrinter*>( _pSetupParent->GetPrinter() )->GetOptions().Clone();
    }

    assert(_pOptions);
    if (!_pOptions)
        return;

    // Create Dialog
    VclPtrInstance<SfxPrintOptionsDialog> pDlg( static_cast<vcl::Window*>(_pSetupParent),
                                                _pViewSh, _pOptions );
    if ( _bHelpDisabled )
        pDlg->DisableHelp();
    if ( pDlg->Execute() == RET_OK )
    {
        delete _pOptions;
        _pOptions = pDlg->GetOptions().Clone();

    }
}

/**
   Internal method for setting the differences between 'pNewPrinter' to the
   current printer. pNewPrinter is either taken over or deleted.
*/
SfxPrinter* SfxViewShell::SetPrinter_Impl( VclPtr<SfxPrinter>& pNewPrinter )
{
    // get current Printer
    SfxPrinter *pDocPrinter = GetPrinter();

    // Evaluate Printer Options
    sal_uInt16 nWhich = GetPool().GetWhich(SID_PRINTER_CHANGESTODOC);
    const SfxFlagItem *pFlagItem = nullptr;
    pDocPrinter->GetOptions().GetItemState( nWhich, false, reinterpret_cast<const SfxPoolItem**>(&pFlagItem) );
    bool bOriToDoc = pFlagItem && (static_cast<SfxPrinterChangeFlags>(pFlagItem->GetValue()) & SfxPrinterChangeFlags::CHG_ORIENTATION);
    bool bSizeToDoc = pFlagItem && (static_cast<SfxPrinterChangeFlags>(pFlagItem->GetValue()) & SfxPrinterChangeFlags::CHG_SIZE);

    // Determine the previous format and size
    Orientation eOldOri = pDocPrinter->GetOrientation();
    Size aOldPgSz = pDocPrinter->GetPaperSizePixel();

    // Determine the new format and size
    Orientation eNewOri = pNewPrinter->GetOrientation();
    Size aNewPgSz = pNewPrinter->GetPaperSizePixel();

    // Determine the changes in page format
    bool bOriChg = (eOldOri != eNewOri) && bOriToDoc;
    bool bPgSzChg = ( aOldPgSz.Height() !=
            ( bOriChg ? aNewPgSz.Width() : aNewPgSz.Height() ) ||
            aOldPgSz.Width() !=
            ( bOriChg ? aNewPgSz.Height() : aNewPgSz.Width() ) ) &&
            bSizeToDoc;

    // Message and Flags for page format, summaries changes
    OUString aMsg;
    SfxPrinterChangeFlags nNewOpt = SfxPrinterChangeFlags::NONE;
    if( bOriChg && bPgSzChg )
    {
        aMsg = SfxResId(STR_PRINT_NEWORISIZE).toString();
        nNewOpt = SfxPrinterChangeFlags::CHG_ORIENTATION | SfxPrinterChangeFlags::CHG_SIZE;
    }
    else if (bOriChg )
    {
        aMsg = SfxResId(STR_PRINT_NEWORI).toString();
        nNewOpt = SfxPrinterChangeFlags::CHG_ORIENTATION;
    }
    else if (bPgSzChg)
    {
        aMsg = SfxResId(STR_PRINT_NEWSIZE).toString();
        nNewOpt = SfxPrinterChangeFlags::CHG_SIZE;
    }

    // Summaries in this variable what has been changed.
    SfxPrinterChangeFlags nChangedFlags = SfxPrinterChangeFlags::NONE;

    // Ask if possible, if page format should be taken over from printer.
    if ( ( bOriChg  || bPgSzChg ) &&
        RET_YES == ScopedVclPtrInstance<MessageDialog>::Create(nullptr, aMsg, VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO)->Execute() )
    {
        // Flags with changes for  <SetPrinter(SfxPrinter*)> are maintained
        nChangedFlags |= nNewOpt;
    }

    // For the MAC to have its "temporary of class String" in next if()
    OUString aTempPrtName = pNewPrinter->GetName();
    OUString aDocPrtName = pDocPrinter->GetName();

    // Was the printer selection changed from Default to Specific
    // or the other way around?
    if ( (aTempPrtName != aDocPrtName) || (pDocPrinter->IsDefPrinter() != pNewPrinter->IsDefPrinter()) )
    {
        nChangedFlags |= SfxPrinterChangeFlags::PRINTER|SfxPrinterChangeFlags::JOBSETUP;
        pDocPrinter = pNewPrinter;
    }
    else
    {
        // Compare extra options
        if ( ! (pNewPrinter->GetOptions() == pDocPrinter->GetOptions()) )
        {
            // Option have changed
            pDocPrinter->SetOptions( pNewPrinter->GetOptions() );
            nChangedFlags |= SfxPrinterChangeFlags::OPTIONS;
        }

        // Compare JobSetups
        JobSetup aNewJobSetup = pNewPrinter->GetJobSetup();
        JobSetup aOldJobSetup = pDocPrinter->GetJobSetup();
        if ( aNewJobSetup != aOldJobSetup )
        {
            nChangedFlags |= SfxPrinterChangeFlags::JOBSETUP;
        }

        // Keep old changed Printer.
        pDocPrinter->SetPrinterProps( pNewPrinter );
        pNewPrinter.disposeAndClear();
    }

    if ( SfxPrinterChangeFlags::NONE != nChangedFlags )
        // SetPrinter will delete the old printer if it changes
        SetPrinter( pDocPrinter, nChangedFlags );
    return pDocPrinter;
}

void SfxViewShell::StartPrint( const uno::Sequence < beans::PropertyValue >& rProps, bool bIsAPI, bool bIsDirect )
{
    assert( pImp->m_xPrinterController.get() == nullptr );

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
    VclPtr<Printer> aPrt;

    const beans::PropertyValue* pVal = rProps.getConstArray();
    for( sal_Int32 i = 0; i < rProps.getLength(); i++ )
    {
        if ( pVal[i].Name == "PrinterName" )
        {
            OUString aPrinterName;
            pVal[i].Value >>= aPrinterName;
            aPrt.reset( VclPtr<Printer>::Create( aPrinterName ) );
            break;
        }
    }

    std::shared_ptr<vcl::PrinterController> xNewController(std::make_shared<SfxPrinterController>(
                                                                               aPrt,
                                                                               aComplete,
                                                                               aSelection,
                                                                               aViewProp,
                                                                               GetRenderable(),
                                                                               bIsAPI,
                                                                               bIsDirect,
                                                                               this,
                                                                               rProps
                                                                               ));
    pImp->m_xPrinterController = xNewController;

    SfxObjectShell *pObjShell = GetObjectShell();
    xNewController->setValue( "JobName",
                        makeAny( OUString( pObjShell->GetTitle() ) ) );
    xNewController->setPrinterModified( mbPrinterSettingsModified );
}

void SfxViewShell::ExecPrint( const uno::Sequence < beans::PropertyValue >& rProps, bool bIsAPI, bool bIsDirect )
{
    StartPrint( rProps, bIsAPI, bIsDirect );
    // FIXME: job setup
    SfxPrinter* pDocPrt = GetPrinter();
    JobSetup aJobSetup = pDocPrt ? pDocPrt->GetJobSetup() : GetJobSetup();
    Printer::PrintJob( GetPrinterController(), aJobSetup );
}

std::shared_ptr< vcl::PrinterController > SfxViewShell::GetPrinterController() const
{
    return pImp->m_xPrinterController;
}

Printer* SfxViewShell::GetActivePrinter() const
{
    return (pImp->m_xPrinterController)
        ?  pImp->m_xPrinterController->getPrinter().get() : nullptr;
}

void SfxViewShell::ExecPrint_Impl( SfxRequest &rReq )
{
    sal_uInt16              nDialogRet = RET_CANCEL;
    VclPtr<SfxPrinter>      pPrinter;
    bool                    bSilent = false;

    // does the function have been called by the user interface or by an API call
    bool bIsAPI = rReq.GetArgs() && rReq.GetArgs()->Count();
    if ( bIsAPI )
    {
        // the function have been called by the API

        // Should it be visible on the user interface,
        // should it launch popup dialogue ?
        const SfxBoolItem* pSilentItem = rReq.GetArg<SfxBoolItem>(SID_SILENT);
        bSilent = pSilentItem && pSilentItem->GetValue();
    }

    // no help button in dialogs if called from the help window
    // (pressing help button would exchange the current page inside the help
    // document that is going to be printed!)
    OUString aHelpFilterName( "writer_web_HTML_help" );
    SfxMedium* pMedium = GetViewFrame()->GetObjectShell()->GetMedium();
    const SfxFilter* pFilter = pMedium ? pMedium->GetFilter() : nullptr;
    bool bPrintOnHelp = ( pFilter && pFilter->GetFilterName() == aHelpFilterName );

    const sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_PRINTDOC: // display the printer selection and properties dialogue : File > Print...
        case SID_PRINTDOCDIRECT: // Print the document directly, without displaying the dialogue
        {
            SfxObjectShell* pDoc = GetObjectShell();

            // derived class may decide to abort this
            if( pDoc == nullptr || !pDoc->QuerySlotExecutable( nId ) )
            {
                rReq.SetReturnValue( SfxBoolItem( 0, false ) );
                return;
            }

            if ( !bSilent && pDoc->QueryHiddenInformation( HiddenWarningFact::WhenPrinting, nullptr ) != RET_YES )
                return;

            // should we print only the selection or the whole document
            const SfxBoolItem* pSelectItem = rReq.GetArg<SfxBoolItem>(SID_SELECTION);
            bool bSelection = ( pSelectItem != nullptr && pSelectItem->GetValue() );
            // detect non api call from writer ( that adds SID_SELECTION ) and reset bIsAPI
            if ( pSelectItem && rReq.GetArgs()->Count() == 1 )
                bIsAPI = false;

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
                        aProps[nProp]. Name = "CopyCount";
                    }
                    else if ( aProps[nProp].Name == "RangeText" )
                    {
                        aProps[nProp]. Name = "Pages";
                    }
                    else if ( aProps[nProp].Name == "Asynchron" )
                    {
                        aProps[nProp]. Name = "Wait";
                        bool bAsynchron = false;
                        aProps[nProp].Value >>= bAsynchron;
                        aProps[nProp].Value <<= (!bAsynchron);
                    }
                    else if ( aProps[nProp].Name == "Silent" )
                    {
                        aProps[nProp]. Name = "MonitorVisible";
                        bool bPrintSilent = false;
                        aProps[nProp].Value >>= bPrintSilent;
                        aProps[nProp].Value <<= (!bPrintSilent);
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
                aProps[nLen].Name = "PrintSelectionOnly";
                aProps[nLen].Value = makeAny( bSelection );
            }
            else // if nId == SID_PRINTDOC ; nothing to do with the previous HACK
            {
                // should the printer selection and properties dialogue display an help button
                aProps[nLen].Name = "HideHelpButton";
                aProps[nLen].Value = makeAny( bPrintOnHelp );
            }

            ExecPrint( aProps, bIsAPI, (nId == SID_PRINTDOCDIRECT) );

            // FIXME: Recording
            rReq.Done();
            break;
        }

        case SID_SETUPPRINTER : // display the printer settings dialogue : File > Printer Settings...
        case SID_PRINTER_NAME : // only for recorded macros
        {
            // get printer and printer settings from the document
            SfxPrinter *pDocPrinter = GetPrinter(true);

            // look for printer in parameters
            const SfxStringItem* pPrinterItem = rReq.GetArg<SfxStringItem>(SID_PRINTER_NAME);
            if ( pPrinterItem )
            {
                // use PrinterName parameter to create a printer
                pPrinter = VclPtr<SfxPrinter>::Create( pDocPrinter->GetOptions().Clone(), pPrinterItem->GetValue() );

                // if printer is unknown, it can't be used - now printer from document will be used
                if ( !pPrinter->IsKnown() )
                    pPrinter.disposeAndClear();
            }

            if ( SID_PRINTER_NAME == nId )
            {
                // just set a recorded printer name
                if ( pPrinter )
                    SetPrinter( pPrinter, SfxPrinterChangeFlags::PRINTER  );
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
                    ScopedVclPtrInstance<MessageDialog>::Create(nullptr, SfxResId(STR_NODEFPRINTER))->Execute();

                rReq.SetReturnValue(SfxBoolItem(0,false));

                break;
            }

            // FIXME: printer isn't used for printing anymore!
            if( pPrinter->IsPrinting() )
            {
                // if printer is busy, abort configuration
                if ( !bSilent )
                    ScopedVclPtrInstance<MessageDialog>::Create(nullptr, SfxResId(STR_ERROR_PRINTER_BUSY), VCL_MESSAGE_INFO)->Execute();
                rReq.SetReturnValue(SfxBoolItem(0,false));

                return;
            }

            // if no arguments are given, retrieve them from a dialog
            if ( !bIsAPI )
            {
                // PrinterDialog needs a temporary printer
                VclPtr<SfxPrinter> pDlgPrinter = pPrinter->Clone();

                // execute PrinterSetupDialog
                VclPtrInstance<PrinterSetupDialog> pPrintSetupDlg( GetWindow() );
                SfxDialogExecutor_Impl* pExecutor = nullptr;

                if (pImp->m_bHasPrintOptions && HasPrintOptionsPage())
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
                        SetPrinter( pPrinter, SfxPrinterChangeFlags::OPTIONS );
                    }
                }

                pPrintSetupDlg.disposeAndClear();
                delete pExecutor;

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

                    // take the changes made in the dialog
                    SetPrinter_Impl( pDlgPrinter );

                    // forget new printer, it was taken over (as pPrinter) or deleted
                    pDlgPrinter = nullptr;
                    mbPrinterSettingsModified = true;
                }
                else
                {
                    // PrinterDialog is used to transfer information on printing,
                    // so it will only be deleted here if dialog was cancelled
                    pDlgPrinter.disposeAndClear();
                    rReq.Ignore();
                }
            }
        }
    }
}

SfxPrinter* SfxViewShell::GetPrinter( bool /*bCreate*/ )
{
    return nullptr;
}

sal_uInt16 SfxViewShell::SetPrinter( SfxPrinter* /*pNewPrinter*/, SfxPrinterChangeFlags /*nDiffFlags*/, bool )
{
    return 0;
}

VclPtr<SfxTabPage> SfxViewShell::CreatePrintOptionsPage
(
    vcl::Window*             /*pParent*/,
    const SfxItemSet&   /*rOptions*/
)
{
    return VclPtr<SfxTabPage>();
}

bool SfxViewShell::HasPrintOptionsPage() const
{
    return false;
}

JobSetup SfxViewShell::GetJobSetup()
{
    return JobSetup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
