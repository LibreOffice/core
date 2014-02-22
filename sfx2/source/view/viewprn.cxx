/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/view/XRenderable.hpp>

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

TYPEINIT1(SfxPrintingHint, SfxViewEventHint);

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
    OUString m_aLastPrintedBy;

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

    
    if( mxRenderable.is() )
    {
        for (sal_Int32 nProp=0; nProp < rProps.getLength(); ++nProp)
            setValue( rProps[nProp].Name, rProps[nProp].Value );

        Sequence< beans::PropertyValue > aRenderOptions( 3 );
        aRenderOptions[0].Name = "ExtraPrintUIOptions";
        aRenderOptions[1].Name = "View" ;
        aRenderOptions[1].Value = i_rViewProp;
        aRenderOptions[2].Name = "IsPrinter";
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
                }
                else if( aRenderParms[i].Name == "NUp" )
                {
                    setValue( aRenderParms[i].Name, aRenderParms[i].Value );
                }
            }
        }
        catch( lang::IllegalArgumentException& )
        {
            
            
        }
    }

    
    setValue( OUString( "IsApi"  ), makeAny( i_bApi ) );
    setValue( OUString( "IsDirect"  ), makeAny( i_bDirect ) );
    setValue( OUString( "IsPrinter"  ), makeAny( sal_True ) );
    setValue( OUString( "View"  ), i_rViewProp );
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
    const beans::PropertyValue* pVal = getValue( OUString( "PrintSelectionOnly"  ) );
    if( pVal )
    {
        sal_Bool bSel = sal_False;
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
    boost::shared_ptr<Printer> pPrinter( getPrinter() );
    if( pPrinter.get() != mpLastPrinter )
    {
        mpLastPrinter = pPrinter.get();
        VCLXDevice* pXDevice = new VCLXDevice();
        pXDevice->SetOutputDevice( mpLastPrinter );
        mxDevice = Reference< awt::XDevice >( pXDevice );
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

        
        if ( m_bOrigStatus && !SvtPrintWarningOptions().IsModifyDocumentOnPrintingAllowed() )
        {
            mpObjectShell->EnableSetModified( sal_False );
            m_bNeedsChange = sal_True;
        }

        
        uno::Reference<document::XDocumentProperties> xDocProps(mpObjectShell->getDocProperties());
        m_aLastPrintedBy = xDocProps->getPrintedBy();
        m_aLastPrinted = xDocProps->getPrintDate();

        xDocProps->setPrintedBy( mpObjectShell->IsUseUserData()
            ? OUString( SvtUserOptions().GetFullName() )
            : OUString() );
        ::DateTime now( ::DateTime::SYSTEM );

        xDocProps->setPrintDate( util::DateTime(
            now.GetNanoSec(), now.GetSec(), now.GetMin(), now.GetHour(),
            now.GetDay(), now.GetMonth(), now.GetYear(), false) );

        SFX_APP()->NotifyEvent( SfxEventHint(SFX_EVENT_PRINTDOC, GlobalEventConfig::GetEventName( STR_EVENT_PRINTDOC ), mpObjectShell ) );
        uno::Sequence < beans::PropertyValue > aOpts;
        aOpts = getJobProperties( aOpts );

        uno::Reference< frame::XController2 > xController;
        if ( mpViewShell )
            xController.set( mpViewShell->GetController(), uno::UNO_QUERY );

        mpObjectShell->Broadcast( SfxPrintingHint(
            view::PrintableState_JOB_STARTED, aOpts, mpObjectShell, xController ) );
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
            case view::PrintableState_JOB_SPOOLING_FAILED :
            case view::PrintableState_JOB_FAILED :
            {
                
                OUString aMsg( SfxResId(STR_NOSTARTPRINTER).toString() );
                if ( !m_bApi )
                    ErrorBox( mpViewShell->GetWindow(), WB_OK | WB_DEF_OK,  aMsg ).Execute();
                
            }
            case view::PrintableState_JOB_ABORTED :
            {
                
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

SfxDialogExecutor_Impl::SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrinterSetupDialog* pParent ) :

    _pViewSh        ( pViewSh ),
    _pSetupParent   ( pParent ),
    _pOptions       ( NULL ),
    _bHelpDisabled  ( sal_False )

{
}

IMPL_LINK_NOARG(SfxDialogExecutor_Impl, Execute)
{
    
    if ( !_pOptions )
    {
        DBG_ASSERT( _pSetupParent, "no dialog parent" );
        if( _pSetupParent )
            _pOptions = ( (SfxPrinter*)_pSetupParent->GetPrinter() )->GetOptions().Clone();
    }

    
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

/**
   Internal method for setting the differences between 'pNewPrinter' to the
   current printer. pNewPrinter is either taken over or deleted.
*/
SfxPrinter* SfxViewShell::SetPrinter_Impl( SfxPrinter *pNewPrinter )
{
    
    SfxPrinter *pDocPrinter = GetPrinter();

    
    bool bOriToDoc = false;
    bool bSizeToDoc = false;
    if ( &pDocPrinter->GetOptions() )
    {
        sal_uInt16 nWhich = GetPool().GetWhich(SID_PRINTER_CHANGESTODOC);
        const SfxFlagItem *pFlagItem = 0;
        pDocPrinter->GetOptions().GetItemState( nWhich, false, (const SfxPoolItem**) &pFlagItem );
        bOriToDoc = pFlagItem ? (pFlagItem->GetValue() & SFX_PRINTER_CHG_ORIENTATION) : sal_False;
        bSizeToDoc = pFlagItem ? (pFlagItem->GetValue() & SFX_PRINTER_CHG_SIZE) : sal_False;
    }

    
    Orientation eOldOri = pDocPrinter->GetOrientation();
    Size aOldPgSz = pDocPrinter->GetPaperSizePixel();

    
    Orientation eNewOri = pNewPrinter->GetOrientation();
    Size aNewPgSz = pNewPrinter->GetPaperSizePixel();

    
    sal_Bool bOriChg = (eOldOri != eNewOri) && bOriToDoc;
    sal_Bool bPgSzChg = ( aOldPgSz.Height() !=
            ( bOriChg ? aNewPgSz.Width() : aNewPgSz.Height() ) ||
            aOldPgSz.Width() !=
            ( bOriChg ? aNewPgSz.Height() : aNewPgSz.Width() ) ) &&
            bSizeToDoc;

    
    OUString aMsg;
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

    
    sal_uInt16 nChangedFlags = 0;

    
    if ( ( bOriChg  || bPgSzChg ) &&
        RET_YES == QueryBox(0, WB_YES_NO | WB_DEF_OK, aMsg).Execute() )
    {
        
        nChangedFlags |= nNewOpt;
    }

    
    OUString aTempPrtName = pNewPrinter->GetName();
    OUString aDocPrtName = pDocPrinter->GetName();

    
    
    if ( (aTempPrtName != aDocPrtName) || (pDocPrinter->IsDefPrinter() != pNewPrinter->IsDefPrinter()) )
    {
        nChangedFlags |= SFX_PRINTER_PRINTER|SFX_PRINTER_JOBSETUP;
        pDocPrinter = pNewPrinter;
    }
    else
    {
        
        if ( ! (pNewPrinter->GetOptions() == pDocPrinter->GetOptions()) )
        {
            
            pDocPrinter->SetOptions( pNewPrinter->GetOptions() );
            nChangedFlags |= SFX_PRINTER_OPTIONS;
        }

        
        JobSetup aNewJobSetup = pNewPrinter->GetJobSetup();
        JobSetup aOldJobSetup = pDocPrinter->GetJobSetup();
        if ( aNewJobSetup != aOldJobSetup )
        {
            nChangedFlags |= SFX_PRINTER_JOBSETUP;
        }

        
        pDocPrinter->SetPrinterProps( pNewPrinter );
        delete pNewPrinter;
    }

    if ( 0 != nChangedFlags )
        
        SetPrinter( pDocPrinter, nChangedFlags );
    return pDocPrinter;
}

void SfxViewShell::ExecPrint( const uno::Sequence < beans::PropertyValue >& rProps, sal_Bool bIsAPI, sal_Bool bIsDirect )
{
    
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
            OUString aPrinterName;
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
    pController->setValue( OUString( "JobName"  ),
                        makeAny( OUString( pObjShell->GetTitle(0) ) ) );

    
    SfxPrinter* pDocPrt = GetPrinter(sal_False);
    JobSetup aJobSetup = pDocPrt ? pDocPrt->GetJobSetup() : GetJobSetup();
    if( bIsDirect )
        aJobSetup.SetValue( OUString( "IsQuickJob"  ),
                            OUString( "true"  ) );

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

    
    sal_Bool bIsAPI = rReq.GetArgs() && rReq.GetArgs()->Count();
    if ( bIsAPI )
    {
        

        
        
        SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem, SID_SILENT, false);
        bSilent = pSilentItem && pSilentItem->GetValue();
    }

    
    
    
    OUString aHelpFilterName( "writer_web_HTML_help" );
    SfxMedium* pMedium = GetViewFrame()->GetObjectShell()->GetMedium();
    const SfxFilter* pFilter = pMedium ? pMedium->GetFilter() : NULL;
    sal_Bool bPrintOnHelp = ( pFilter && pFilter->GetFilterName() == aHelpFilterName );

    const sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_PRINTDOC: 
        case SID_PRINTDOCDIRECT: 
        {
            SfxObjectShell* pDoc = GetObjectShell();

            
            if( pDoc == NULL || !pDoc->QuerySlotExecutable( nId ) )
            {
                rReq.SetReturnValue( SfxBoolItem( 0, false ) );
                return;
            }

            if ( !bSilent && pDoc->QueryHiddenInformation( WhenPrinting, NULL ) != RET_YES )
                return;

            
            SFX_REQUEST_ARG(rReq, pSelectItem, SfxBoolItem, SID_SELECTION, false);
            sal_Bool bSelection = ( pSelectItem != NULL && pSelectItem->GetValue() );
            
            if ( pSelectItem && rReq.GetArgs()->Count() == 1 )
                bIsAPI = sal_False;

            uno::Sequence < beans::PropertyValue > aProps;
            if ( bIsAPI )
            {
                
                
                
                
                
                
                
                
                
                
                

                
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
                        sal_Bool bAsynchron = sal_False;
                        aProps[nProp].Value >>= bAsynchron;
                        aProps[nProp].Value <<= (sal_Bool) (!bAsynchron);
                    }
                    else if ( aProps[nProp].Name == "Silent" )
                    {
                        aProps[nProp]. Name = "MonitorVisible";
                        sal_Bool bPrintSilent = sal_False;
                        aProps[nProp].Value >>= bPrintSilent;
                        aProps[nProp].Value <<= (sal_Bool) (!bPrintSilent);
                    }
                }
            }

            
            
            sal_Int32 nLen = aProps.getLength();
            aProps.realloc( nLen + 1 );

            
            
            
            
            
            if( nId == SID_PRINTDOCDIRECT )
            {
                aProps[nLen].Name = "PrintSelectionOnly";
                aProps[nLen].Value = makeAny( bSelection );
            }
            else 
            {
                
                aProps[nLen].Name = "HideHelpButton";
                aProps[nLen].Value = makeAny( bPrintOnHelp );
            }

            ExecPrint( aProps, bIsAPI, (nId == SID_PRINTDOCDIRECT) );

            
            rReq.Done();
            break;
        }

        case SID_SETUPPRINTER : 
        case SID_PRINTER_NAME : 
        {
            
            SfxPrinter *pDocPrinter = GetPrinter(sal_True);

            
            SFX_REQUEST_ARG( rReq, pPrinterItem, SfxStringItem, SID_PRINTER_NAME, false );
            if ( pPrinterItem )
            {
                
                pPrinter = new SfxPrinter( pDocPrinter->GetOptions().Clone(), ((const SfxStringItem*) pPrinterItem)->GetValue() );

                
                if ( !pPrinter->IsKnown() )
                    DELETEZ(pPrinter);
            }

            if ( SID_PRINTER_NAME == nId )
            {
                
                if ( pPrinter )
                    SetPrinter( pPrinter, SFX_PRINTER_PRINTER  );
                return;
            }

            
            if ( !pPrinter )
                
                pPrinter = pDocPrinter;

            if( !pPrinter || !pPrinter->IsValid() )
            {
                
                if ( !bSilent )
                    ErrorBox( NULL, WB_OK | WB_DEF_OK, SfxResId(STR_NODEFPRINTER).toString() ).Execute();

                rReq.SetReturnValue(SfxBoolItem(0,false));

                break;
            }

            
            if( pPrinter->IsPrinting() )
            {
                
                if ( !bSilent )
                    InfoBox( NULL, SfxResId(STR_ERROR_PRINTER_BUSY).toString() ).Execute();
                rReq.SetReturnValue(SfxBoolItem(0,false));

                return;
            }

            
            if ( !bIsAPI )
            {
                
                SfxPrinter* pDlgPrinter = pPrinter->Clone();
                nDialogRet = 0;

                
                PrinterSetupDialog* pPrintSetupDlg = new PrinterSetupDialog( GetWindow() );

                if (pImp->m_bHasPrintOptions && HasPrintOptionsPage())
                {
                    
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
                        
                        pDlgPrinter->SetOptions( *pExecutor->GetOptions() );
                    else
                    {
                        pPrinter->SetOptions( *pExecutor->GetOptions() );
                        SetPrinter( pPrinter, SFX_PRINTER_OPTIONS );
                    }
                }

                DELETEZ( pPrintSetupDlg );

                
                rReq.Ignore();

                if ( nDialogRet == RET_OK )
                {
                    if ( pPrinter->GetName() != pDlgPrinter->GetName() )
                    {
                        
                        SfxRequest aReq( GetViewFrame(), SID_PRINTER_NAME );
                        aReq.AppendItem( SfxStringItem( SID_PRINTER_NAME, pDlgPrinter->GetName() ) );
                        aReq.Done();
                    }

                    
                    SetPrinter_Impl( pDlgPrinter );

                    
                    pDlgPrinter = NULL;

                }
                else
                {
                    
                    
                    DELETEZ( pDlgPrinter );
                    rReq.Ignore();
                }
            }
        }
    }
}

SfxPrinter* SfxViewShell::GetPrinter( sal_Bool /*bCreate*/ )
{
    return 0;
}

sal_uInt16 SfxViewShell::SetPrinter( SfxPrinter* /*pNewPrinter*/, sal_uInt16 /*nDiffFlags*/, bool )
{
    return 0;
}

SfxTabPage* SfxViewShell::CreatePrintOptionsPage
(
    Window*             /*pParent*/,
    const SfxItemSet&   /*rOptions*/
)
{
    return 0;
}

bool SfxViewShell::HasPrintOptionsPage() const
{
    return false;
}

JobSetup SfxViewShell::GetJobSetup() const
{
    return JobSetup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
