/*************************************************************************
 *
 *  $RCSfile: prnmon.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#pragma hdrstop

#include "prnmon.hxx"
#include "viewsh.hxx"
#include "viewfrm.hxx"
#include "objsh.hxx"
#include "docfile.hxx"
#include "sfxtypes.hxx"
#include "progress.hxx"
#include "desrupt.hxx"
#include "bindings.hxx"
#include "sfxresid.hxx"

#include "view.hrc"

//------------------------------------------------------------------------

#define SFX_TITLE_MAXLEN_PRINTMONITOR   22

//------------------------------------------------------------------------

struct SfxPrintMonitor_Impl: public ModelessDialog
{
    SfxPrintMonitor_Impl( Window *pParent );

    FixedText               aDocName;
    FixedText               aPrinting;
    FixedText               aPrinter;
    FixedText               aPrintInfo;
    CancelButton            aCancel;
};

//-------------------------------------------------------------------------

struct SfxPrintProgress_Impl
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
    AsynchronLink           aDeleteLink;

private:
    DECL_LINK( CancelHdl, Button * );
    DECL_STATIC_LINK( SfxPrintProgress_Impl, DeleteHdl, SfxPrintProgress * );

public:
    SfxPrintProgress_Impl( SfxViewShell* pTheViewShell, SfxPrinter* pThePrinter );
    ~SfxPrintProgress_Impl();

    void                    Delete( SfxPrintProgress* pAntiImpl ) { aDeleteLink.Call( pAntiImpl ); }
    SfxViewShell*           GetViewShell() const { return pViewShell; }
    BOOL                    SetPage( USHORT nPage, const String &rPage );
};

//------------------------------------------------------------------------

SfxPrintMonitor_Impl::SfxPrintMonitor_Impl( Window* pParent ) :

    ModelessDialog( pParent, SfxResId( DLG_PRINTMONITOR ) ),

    aDocName    ( this, ResId( FT_DOCNAME ) ),
    aPrinting   ( this, ResId( FT_PRINTING ) ),
    aPrinter    ( this, ResId( FT_PRINTER ) ),
    aPrintInfo  ( this, ResId( FT_PRINTINFO ) ),
    aCancel     ( this, ResId( PB_CANCELPRNMON ) )

{
    FreeResource();
}

//------------------------------------------------------------------------

IMPL_STATIC_LINK( SfxPrintProgress_Impl, DeleteHdl, SfxPrintProgress*, pAntiImpl )
{
    delete pAntiImpl;
    return 0;
}

//------------------------------------------------------------------------

SfxPrintProgress_Impl::SfxPrintProgress_Impl( SfxViewShell* pTheViewShell,
                                              SfxPrinter* pThePrinter ) :

    pViewShell          ( pTheViewShell ),
    pPrinter            ( pThePrinter ),
    pOldPrinter         ( NULL ),
    bRunning            ( TRUE ),
    bDeleteOnEndPrint   ( FALSE ),
    bCancel             ( FALSE ),
    bCallbacks          ( FALSE ),
    bOldEnablePrintFile ( FALSE ),
    nLastPage           ( 0 ),
    aDeleteLink         ( STATIC_LINK( this, SfxPrintProgress_Impl, DeleteHdl ) )

{
    Window* pParent =
        pTheViewShell->GetWindow()->IsReallyVisible() ? pTheViewShell->GetWindow() : NULL;
    pMonitor = new SfxPrintMonitor_Impl( pParent );
    pMonitor->aDocName.SetText(
        pViewShell->GetViewFrame()->GetObjectShell()->GetTitle( SFX_TITLE_MAXLEN_PRINTMONITOR ) );
    pMonitor->aPrinter.SetText( pViewShell->GetPrinter()->GetName() );
    pMonitor->aCancel.SetClickHdl( LINK( this, SfxPrintProgress_Impl, CancelHdl ) );
}

//------------------------------------------------------------------------

SfxPrintProgress_Impl::~SfxPrintProgress_Impl()
{
    if ( pMonitor )
    {
        pMonitor->Hide(); // sieht optisch besser aus, wenn alles auf einmal verschwindet
        delete pMonitor;
    }
}

//------------------------------------------------------------------------

BOOL SfxPrintProgress_Impl::SetPage( USHORT nPage, const String &rPage )
{
    // wurde der Druckauftrag abgebrochen?
    if ( bCancel || !pMonitor )
        return FALSE;

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

IMPL_LINK_INLINE_START( SfxPrintProgress_Impl, CancelHdl, Button *, pButton )
{
    if ( pMonitor )
        pMonitor->Hide();
    pViewShell->GetPrinter()->AbortJob();
    bCancel = TRUE;
    return 0;
}
IMPL_LINK_INLINE_END( SfxPrintProgress_Impl, CancelHdl, Button *, pButton )

//--------------------------------------------------------------------

SfxPrintProgress::SfxPrintProgress( SfxViewShell* pViewSh, FASTBOOL bShow )
:   SfxProgress( pViewSh->GetViewFrame()->GetObjectShell(),
                 String(SfxResId(STR_PRINTING)), 1, FALSE ),
    pImp( new SfxPrintProgress_Impl( pViewSh, pViewSh->GetPrinter() ) )
{
    // Callback fuer Fehler und EndPrint setzen
    pImp->pPrinter->SetEndPrintHdl(
                LINK( this, SfxPrintProgress, EndPrintNotify ));
    pImp->pPrinter->SetErrorHdl(
                LINK( this, SfxPrintProgress, PrintErrorNotify ));
    pImp->bCallbacks = TRUE;

    pImp->pViewShell->GetViewFrame()->GetFrame()->Lock_Impl(TRUE);
    pImp->bShow = bShow;
    Lock();
}

//--------------------------------------------------------------------

SfxPrintProgress::~SfxPrintProgress()
{
    // k"onnte auch schon weg sein (in EndPrintNotify)
    DELETEZ(pImp->pMonitor);

    // ggf. Callbacks entfermen
    if ( pImp->bCallbacks )
    {
        pImp->pPrinter->SetEndPrintHdl( Link() );
        pImp->pPrinter->SetErrorHdl( Link() );
        pImp->bCallbacks = FALSE;
    }

    // ggf. vorherigen Drucker wieder einsetzen
    if ( pImp->pOldPrinter )
        pImp->pViewShell->SetPrinter( pImp->pOldPrinter, SFX_PRINTER_PRINTER );
    else
        // ggf. vorherigen Print-To-File-Status zuruecksetzen
        pImp->pViewShell->GetPrinter()->EnablePrintFile(
                pImp->bOldEnablePrintFile );

    // EndPrint-Notification an Frame
    pImp->pViewShell->GetViewFrame()->GetFrame()->Lock_Impl(FALSE);

    delete pImp;
}

//--------------------------------------------------------------------

BOOL SfxPrintProgress::SetState( ULONG nVal, ULONG nNewRange )
{
#ifndef MAC
    // auf dem MAC kommt einer vom Betriebssystem
    if ( pImp->bShow )
    {
        pImp->bShow = FALSE;
        pImp->pMonitor->Show();
        pImp->pMonitor->Update();
    }
#endif

    return pImp->SetPage( (USHORT)nVal, GetStateText_Impl() ) &&
           SfxProgress::SetState( nVal, nNewRange );
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

IMPL_LINK_INLINE_START( SfxPrintProgress, PrintErrorNotify, void *, pvoid )
{
    if ( pImp->pMonitor )
        pImp->pMonitor->Hide();
    pImp->pPrinter->AbortJob();
    InfoBox( pImp->GetViewShell()->GetWindow(),
             String( SfxResId(STR_ERROR_PRINT) ) ).Execute();
    return 0;
}
IMPL_LINK_INLINE_END( SfxPrintProgress, PrintErrorNotify, void *, pvoid )

//------------------------------------------------------------------------

IMPL_LINK( SfxPrintProgress, EndPrintNotify, void *, pvoid )
{
    if ( pImp->pMonitor )
        pImp->pMonitor->Hide();

    // Slots enablen
    pImp->pViewShell->Invalidate( SID_PRINTDOC );
    pImp->pViewShell->Invalidate( SID_PRINTDOCDIRECT );
    pImp->pViewShell->Invalidate( SID_SETUPPRINTER );

    // . . . falls der Printer im System umgestellt wurde, hier Aenderung
    // nachziehen.
    //! if( pMDI->IsPrinterChanged() ) pMDI->Changed( 0L );

    // Callbacks rausnehmen
    pImp->pPrinter->SetEndPrintHdl( Link() );
    pImp->pPrinter->SetErrorHdl( Link() );
    pImp->bCallbacks = FALSE;

    // ggf. alten Printer wieder einsetzen
    if ( pImp->pOldPrinter )
    {
        // Fix #59613#: niemals den aktuellen Printer synchron abschiessen !
        // Da sowieso immer bDeleteOnEndPrint gesetzt wird, wird der der Drucker im
        // dtor vom Printprogress ( dann aber asynchron !! ) zur"uckgesetzt.
/*
        pImp->pViewShell->SetPrinter( pImp->pOldPrinter, SFX_PRINTER_PRINTER );
        pImp->pOldPrinter = 0;
        pImp->pPrinter = 0;
 */
    }
    else
        // ggf. vorherigen Print-To-File-Status zuruecksetzen
        pImp->pViewShell->GetPrinter()->EnablePrintFile( pImp->bOldEnablePrintFile );

    // lief der Drucker im Thread?
    if ( pImp->bDeleteOnEndPrint )
    {
        // Dialog sofort l"oschen sonst wird ggf. das MDI vorher geschlossen
        DELETEZ(pImp->pMonitor);

        // Progress per PostMessage zerst"oren, nicht sofort sonst GPF
        pImp->Delete( this );
    }
    else
    {
        DBG_ASSERT( !pImp->pOldPrinter, "Printer konnte nicht korrekt restauriert werden!" );
        pImp->bRunning = FALSE;
    }

    return 0;
}

//------------------------------------------------------------------------

void SfxPrintProgress::DeleteOnEndPrint()
{
    UnLock(); // jetzt schon, wg. Drucken im Thread
#ifndef WIN
    // da das Drucken im 'Thread' unter Windows zu undefiniert ist bleibt der
    // Print-Monitor dort stehen, auf den anderen Plattformen kann man dann
    // weiterarbeiten, also kommt das Teil weg
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


