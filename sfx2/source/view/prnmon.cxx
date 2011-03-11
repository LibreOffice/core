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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <com/sun/star/view/PrintableState.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <vcl/fixed.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/asynclink.hxx>

#include <unotools/printwarningoptions.hxx>
#include <svl/eitem.hxx>

#include <sfx2/prnmon.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include "sfxtypes.hxx"
#include <sfx2/progress.hxx>
#include <sfx2/bindings.hxx>
#include "sfxresid.hxx"
#include <sfx2/event.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>

#include "view.hrc"

//------------------------------------------------------------------------

#define SFX_TITLE_MAXLEN_PRINTMONITOR   22

//------------------------------------------------------------------------

struct SfxPrintMonitor_Impl: public ModelessDialog
{

    SfxPrintMonitor_Impl( Window *pParent, SfxViewShell *rpViewShell );
    virtual BOOL Close();

    SfxViewShell*           pViewShell;
    FixedText               aDocName;
    FixedText               aPrinting;
    FixedText               aPrinter;
    FixedText               aPrintInfo;
    CancelButton            aCancel;
};

//-------------------------------------------------------------------------

struct SfxPrintProgress_Impl : public SfxListener
{
    SfxPrintMonitor_Impl*   pMonitor;
    SfxViewShell*           pViewShell;
    SfxPrinter*             pPrinter;
    SfxPrinter*             pOldPrinter;
    USHORT                  nLastPage;
    BOOL                    bRunning;
    BOOL                    bCancel;
    BOOL                    bDeleteOnEndPrint;
    BOOL                    bShow;
    BOOL                    bCallbacks;
    BOOL                    bOldEnablePrintFile;
    BOOL                    bOldFlag;
    BOOL                    bRestoreFlag;
    BOOL                    bAborted;
    svtools::AsynchronLink  aDeleteLink;
    Link                    aCancelHdl;

private:
    DECL_LINK( CancelHdl, Button * );
    DECL_STATIC_LINK( SfxPrintProgress_Impl, DeleteHdl, SfxPrintProgress * );

public:
    SfxPrintProgress_Impl( SfxViewShell* pTheViewShell, SfxPrinter* pThePrinter );
    ~SfxPrintProgress_Impl();

    void                    Delete( SfxPrintProgress* pAntiImpl ) { aDeleteLink.Call( pAntiImpl ); }
    SfxViewShell*           GetViewShell() const { return pViewShell; }
    BOOL                    SetPage( USHORT nPage, const String &rPage );
    void                    CreateMonitor();
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

void SfxPrintProgress_Impl::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    SfxPrintingHint* pPrintHint = PTR_CAST( SfxPrintingHint, &rHint );
    if ( pPrintHint )
    {
        if ( pPrintHint->GetWhich() == -2 )
            CancelHdl(0);
    }
}

//------------------------------------------------------------------------

SfxPrintMonitor_Impl::SfxPrintMonitor_Impl( Window* pParent, SfxViewShell* rpViewShell ) :
    ModelessDialog( pParent, SfxResId( DLG_PRINTMONITOR ) ),
    pViewShell  ( rpViewShell ),
    aDocName    ( this, SfxResId( FT_DOCNAME ) ),
    aPrinting   ( this, SfxResId( FT_PRINTING ) ),
    aPrinter    ( this, SfxResId( FT_PRINTER ) ),
    aPrintInfo  ( this, SfxResId( FT_PRINTINFO ) ),
    aCancel     ( this, SfxResId( PB_CANCELPRNMON ) )
{
    if( rpViewShell->GetPrinter()->GetCapabilities( PRINTER_CAPABILITIES_EXTERNALDIALOG ) != 0 )
    {
        String aPrep( SfxResId( STR_FT_PREPARATION ) );
        aPrinting.SetText( aPrep );
        aPrinter.Show( FALSE );
    }
    FreeResource();
}

//------------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( SfxPrintProgress_Impl, DeleteHdl, SfxPrintProgress*, pAntiImpl )
{
    delete pAntiImpl;
    return 0;
}

void actualizePrintCancelState(CancelButton& rButton, const SfxObjectShell* pShell)
{
    sal_Bool bEnableCancelButton = sal_True;
    if (pShell)
        bEnableCancelButton = pShell->Stamp_GetPrintCancelState();

    if (!bEnableCancelButton)
        rButton.Disable();
    else
        rButton.Enable();
}

//------------------------------------------------------------------------

SfxPrintProgress_Impl::SfxPrintProgress_Impl( SfxViewShell* pTheViewShell,
                                              SfxPrinter* pThePrinter ) :

    pMonitor            ( 0 ),
    pViewShell          ( pTheViewShell ),
    pPrinter            ( pThePrinter ),
    pOldPrinter         ( NULL ),
    nLastPage           ( 0 ),
    bRunning            ( TRUE ),
    bCancel             ( FALSE ),
    bDeleteOnEndPrint   ( FALSE ),
    bShow               ( FALSE ),
    bCallbacks          ( FALSE ),
    bOldEnablePrintFile ( FALSE ),
    bOldFlag            ( TRUE ),
    bRestoreFlag        ( FALSE ),
    bAborted            ( FALSE ),
    aDeleteLink         ( STATIC_LINK( this, SfxPrintProgress_Impl, DeleteHdl ) )
{
    StartListening( *pViewShell->GetObjectShell() );
}

void SfxPrintProgress_Impl::CreateMonitor()
{
    // mark monitor to been shown in first status indication
    bShow = TRUE;

    if ( !pMonitor )
    {
        Window* pParent = pViewShell->GetWindow();
        pMonitor = new SfxPrintMonitor_Impl( pParent, pViewShell );
        pMonitor->aDocName.SetText( pViewShell->GetObjectShell()->GetTitle( SFX_TITLE_MAXLEN_PRINTMONITOR ) );
        pMonitor->aPrinter.SetText( pViewShell->GetPrinter()->GetName() );

        // Stampit enable/dsiable cancel button
        actualizePrintCancelState( pMonitor->aCancel, pViewShell->GetObjectShell() );
        pMonitor->aCancel.SetClickHdl( LINK( this, SfxPrintProgress_Impl, CancelHdl ) );
    }
}

//------------------------------------------------------------------------

SfxPrintProgress_Impl::~SfxPrintProgress_Impl()
{
}

//------------------------------------------------------------------------

BOOL SfxPrintProgress_Impl::SetPage( USHORT nPage, const String &rPage )
{
    // Was the print job canceled?
    if ( bCancel || !pMonitor )
        return FALSE;

    // Stampit enable/dsiable cancel button
    actualizePrintCancelState(pMonitor->aCancel, pViewShell->GetObjectShell());

    nLastPage = nPage;
    String aStrPrintInfo = String( SfxResId( STR_PAGE ) );
    if ( !rPage.Len() )
        aStrPrintInfo += String::CreateFromInt32( nLastPage );
    else
        aStrPrintInfo += rPage;
    pMonitor->aPrintInfo.SetText( aStrPrintInfo );
    pMonitor->Update();
    return TRUE;
}

//------------------------------------------------------------------------

IMPL_LINK( SfxPrintProgress_Impl, CancelHdl, Button *, EMPTYARG )
{
    if ( !pViewShell->GetPrinter()->IsJobActive() && pViewShell->GetPrinter()->IsPrinting() )
        // we are still in StartJob, cancelling now might lead to a crash
        return 0;

    if ( pMonitor )
        pMonitor->Hide();

    pViewShell->GetObjectShell()->Broadcast( SfxPrintingHint( com::sun::star::view::PrintableState_JOB_ABORTED, NULL, NULL ) );
    pViewShell->GetPrinter()->AbortJob();
    bCancel = TRUE;

    if ( aCancelHdl.IsSet() )
        aCancelHdl.Call( this );

    bAborted = TRUE;
    return 0;
}

BOOL SfxPrintMonitor_Impl::Close()
{
    BOOL bAgree = pViewShell ? pViewShell->GetObjectShell()->Stamp_GetPrintCancelState() : TRUE;
    if (!bAgree)
        return FALSE;
    else
        return ModelessDialog::Close();
}

//--------------------------------------------------------------------

SfxPrintProgress::SfxPrintProgress( SfxViewShell* pViewSh, bool bShow )
:   SfxProgress( pViewSh->GetViewFrame()->GetObjectShell(),
                 String(SfxResId(STR_PRINTING)), 1, FALSE ),
    pImp( new SfxPrintProgress_Impl( pViewSh, pViewSh->GetPrinter() ) )
{
    SfxObjectShell* pDoc = pViewSh->GetObjectShell();
    SFX_ITEMSET_ARG( pDoc->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_HIDDEN, FALSE );
    if ( pItem && pItem->GetValue() )
        bShow = FALSE;

    if ( bShow )
        pImp->CreateMonitor();

    Lock();
    if ( !SvtPrintWarningOptions().IsModifyDocumentOnPrintingAllowed() )
    {
        pImp->bRestoreFlag = TRUE;
        pImp->bOldFlag = pDoc->IsEnableSetModified();
        if ( pImp->bOldFlag )
            pDoc->EnableSetModified( FALSE );
    }
}

//--------------------------------------------------------------------

SfxPrintProgress::~SfxPrintProgress()
{
    // Can already have been deleted (in EndPrintNotify)
    DELETEZ(pImp->pMonitor);

    // If possible remove Callbacks
    if ( pImp->bCallbacks )
    {
        // pImp->pPrinter->SetEndPrintHdl( Link() );
        pImp->pPrinter->SetErrorHdl( Link() );
        pImp->bCallbacks = FALSE;
    }

    // If possible use the previously used printer
    if ( pImp->pOldPrinter )
        pImp->pViewShell->SetPrinter( pImp->pOldPrinter, SFX_PRINTER_PRINTER );
    else
        // If possible reset Print-To-File-Status
        pImp->pViewShell->GetPrinter()->EnablePrintFile( pImp->bOldEnablePrintFile );

    // EndPrint-Notification an Frame
    //pImp->pViewShell->GetViewFrame()->GetFrame().Lock_Impl(FALSE);
    pImp->EndListening( *(pImp->pViewShell->GetObjectShell()) );

    // the following call might destroy the view or even the document
    pImp->pViewShell->CheckOwnerShip_Impl();
    delete pImp;
}

//--------------------------------------------------------------------

BOOL SfxPrintProgress::SetState( ULONG nValue, ULONG nNewRange )
{
    if ( pImp->bShow )
    {
        pImp->bShow = FALSE;
        if ( pImp->pMonitor )
        {
            pImp->pMonitor->Show();
            pImp->pMonitor->Update();
        }
    }

    return pImp->SetPage( (USHORT)nValue, GetStateText_Impl() ) &&
           SfxProgress::SetState( nValue, nNewRange );
}

//--------------------------------------------------------------------

void SfxPrintProgress::SetText( const String& rText )
{
    if ( pImp->pMonitor )
    {
        pImp->pMonitor->SetText( rText );
        pImp->pMonitor->Update();
    }
    SfxProgress::SetText( rText );
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxPrintProgress, PrintErrorNotify, void *, EMPTYARG )
{
    if ( pImp->pMonitor )
        pImp->pMonitor->Hide();
    // AbortJob calls EndPrint hdl, so do not delete pImp just now
    BOOL bWasDeleteOnEndPrint = pImp->bDeleteOnEndPrint;
    pImp->bDeleteOnEndPrint = FALSE;
    pImp->pPrinter->AbortJob();
    InfoBox( pImp->GetViewShell()->GetWindow(),
             String( SfxResId(STR_ERROR_PRINT) ) ).Execute();
    if ( pImp->bRestoreFlag && pImp->pViewShell->GetObjectShell()->IsEnableSetModified() != pImp->bOldFlag )
        pImp->pViewShell->GetObjectShell()->EnableSetModified( pImp->bOldFlag );
    pImp->GetViewShell()->GetObjectShell()->Broadcast( SfxPrintingHint( com::sun::star::view::PrintableState_JOB_FAILED, NULL, NULL ) );
    // now we can clean up like normally EndPrint hdl does
    if( bWasDeleteOnEndPrint )
    {
        DELETEZ(pImp->pMonitor);
        delete this;
    }
    return 0;
}
IMPL_LINK_INLINE_END( SfxPrintProgress, PrintErrorNotify, void *, EMPTYARG )

//------------------------------------------------------------------------

IMPL_LINK( SfxPrintProgress, StartPrintNotify, void *, EMPTYARG )
{
    SfxObjectShell *pObjShell = pImp->pViewShell->GetObjectShell();
    SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_PRINTDOC, GlobalEventConfig::GetEventName( STR_EVENT_PRINTDOC ), pObjShell));
    pObjShell->Broadcast( SfxPrintingHint( com::sun::star::view::PrintableState_JOB_STARTED, NULL, NULL ) );
    return 0;
}

IMPL_LINK( SfxPrintProgress, EndPrintNotify, void *, EMPTYARG )
{
    if ( pImp->pMonitor )
        pImp->pMonitor->Hide();

    SfxViewShell* pViewShell = pImp->pViewShell;

    // Slots enablen
    pViewShell->Invalidate( SID_PRINTDOC );
    pViewShell->Invalidate( SID_PRINTDOCDIRECT );
    pViewShell->Invalidate( SID_SETUPPRINTER );

    // Check out Callbacks
    pImp->pPrinter->SetErrorHdl( Link() );
    pImp->bCallbacks = FALSE;

    // If possible insert the old Printer again.
    if ( pImp->pOldPrinter )
    {
    }
    else
        // If possible reset previous Print-To-File-Status.
        pViewShell->GetPrinter()->EnablePrintFile( pImp->bOldEnablePrintFile );

    // it is possible that after printing the document or view is deleted (because the VieShell got the ownership)
    // so first clean up
    if ( pImp->bRestoreFlag && pViewShell->GetObjectShell()->IsEnableSetModified() != pImp->bOldFlag )
        pViewShell->GetObjectShell()->EnableSetModified( TRUE );

    pViewShell->GetObjectShell()->Broadcast( SfxPrintingHint( com::sun::star::view::PrintableState_JOB_COMPLETED, NULL, NULL ) );
    if ( pImp->bDeleteOnEndPrint )
    {
        DELETEZ(pImp->pMonitor);
        delete this;
    }
    else
    {
        DBG_ASSERT( !pImp->pOldPrinter, "Unable to restore printer!" );
        pImp->bRunning = FALSE;
    }

    return 0;
}

//------------------------------------------------------------------------

void SfxPrintProgress::DeleteOnEndPrint()
{
    UnLock(); // Already now because printer in Thread.
#ifndef WNT
    // As the print in the 'Thread' is undefined on Windows, the Print-Monitor
    // will get stuck here, on the other platforms the work can continue,
    // thus this part is removed.
    DELETEZ( pImp->pMonitor );
#endif

    pImp->bDeleteOnEndPrint = TRUE;
    if ( !pImp->bRunning )
        delete this;
}

//------------------------------------------------------------------------

void SfxPrintProgress::RestoreOnEndPrint( SfxPrinter *pOldPrinter,
                                          BOOL bOldEnablePrintFile )
{
    pImp->pOldPrinter = pOldPrinter;
    pImp->bOldEnablePrintFile = bOldEnablePrintFile;
}

//------------------------------------------------------------------------

void SfxPrintProgress::RestoreOnEndPrint( SfxPrinter *pOldPrinter )
{
    RestoreOnEndPrint( pOldPrinter, FALSE );
}

//------------------------------------------------------------------------

void SfxPrintProgress::SetCancelHdl( const Link& aCancelHdl )
{
    pImp->aCancelHdl = aCancelHdl;
}

BOOL SfxPrintProgress::IsAborted() const
{
    return pImp->bAborted;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
