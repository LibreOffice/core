/*************************************************************************
 *
 *  $RCSfile: prnsetup.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _VCL_PRINT_HXX
#include <vcl/print.hxx>
#endif

#pragma hdrstop

#include <svtdata.hxx>
#include "prnsetup.hrc"
#include <prnsetup.hxx>

// =======================================================================

void ImplFillPrnDlgListBox( const Printer* pPrinter,
                            ListBox* pBox, PushButton* pPropBtn )
{
    ImplFreePrnDlgListBox( pBox );

    USHORT nCount = Printer::GetQueueCount();
    if ( nCount )
    {
        for( USHORT i = 0; i < nCount; i++ )
        {
            const QueueInfo& rInfo = Printer::GetQueueInfo( i, FALSE );
            USHORT nPos = pBox->InsertEntry( rInfo.GetPrinterName() );
            if ( nPos != LISTBOX_ERROR )
                pBox->SetEntryData( nPos, new QueueInfo( rInfo ) );
        }

        pBox->SelectEntry( pPrinter->GetName() );
    }

    pBox->Enable( nCount != 0 );
    pPropBtn->Enable( pPrinter->HasSupport( SUPPORT_SETUPDIALOG ) );
}

// -----------------------------------------------------------------------

void ImplFreePrnDlgListBox( ListBox* pBox, BOOL bClear )
{
    USHORT nEntryCount = pBox->GetEntryCount();
    for ( USHORT i = 0; i < nEntryCount; i++ )
        delete (QueueInfo*)pBox->GetEntryData( i );

    if ( bClear )
        pBox->Clear();
}

// -----------------------------------------------------------------------

Printer* ImplPrnDlgListBoxSelect( ListBox* pBox, PushButton* pPropBtn,
                                  Printer* pPrinter, Printer* pTempPrinter )
{
    if ( pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        const QueueInfo& rInfo = *((QueueInfo*)(pBox->GetEntryData( pBox->GetSelectEntryPos() )));

        if ( !pTempPrinter )
        {
            if ( (pPrinter->GetName() == rInfo.GetPrinterName()) &&
                 (pPrinter->GetDriverName() == rInfo.GetDriver()) )
                pTempPrinter = new Printer( pPrinter->GetJobSetup() );
            else
                pTempPrinter = new Printer( rInfo );
        }
        else
        {
            if ( (pTempPrinter->GetName() != rInfo.GetPrinterName()) ||
                 (pTempPrinter->GetDriverName() != rInfo.GetDriver()) )
            {
                delete pTempPrinter;
                pTempPrinter = new Printer( rInfo );
            }
        }

        pPropBtn->Enable( pTempPrinter->HasSupport( SUPPORT_SETUPDIALOG ) );
    }
    else
        pPropBtn->Disable();

    return pTempPrinter;
}

// -----------------------------------------------------------------------

Printer* ImplPrnDlgUpdatePrinter( Printer* pPrinter, Printer* pTempPrinter )
{
    XubString aPrnName;
    if ( pTempPrinter )
        aPrnName = pTempPrinter->GetName();
    else
        aPrnName = pPrinter->GetName();

    BOOL    bFound = FALSE;
    USHORT  nCount = Printer::GetQueueCount();
    for( USHORT i = 0; i < nCount; i++ )
    {
        if ( aPrnName == Printer::GetQueueInfo( i, FALSE ).GetPrinterName() )
        {
            bFound = TRUE;
            break;
        }
    }

    if ( !bFound )
    {
        if ( pTempPrinter )
            delete pTempPrinter;
        pTempPrinter = new Printer;
    }

    return pTempPrinter;
}

// -----------------------------------------------------------------------

void ImplPrnDlgUpdateQueueInfo( ListBox* pBox, QueueInfo& rInfo )
{
    if ( pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        rInfo = *((QueueInfo*)(pBox->GetEntryData( pBox->GetSelectEntryPos() )));

        USHORT nCount = Printer::GetQueueCount();
        for( USHORT i = 0; i < nCount; i++ )
        {
            const QueueInfo& rTempInfo = Printer::GetQueueInfo( i, FALSE );
            if ( (rInfo.GetPrinterName() == rTempInfo.GetPrinterName()) &&
                 (rInfo.GetDriver() == rTempInfo.GetDriver()) )
            {
                rInfo = Printer::GetQueueInfo( i );
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

static void ImplPrnDlgAddString( XubString& rStr, const XubString& rAddStr )
{
    if ( rStr.Len() )
        rStr.AppendAscii( "; " );
    rStr += rAddStr;
}

// -----------------------------------------------------------------------

static void ImplPrnDlgAddResString( XubString& rStr, USHORT nResId )
{
    SvtResId aResId( nResId );
    XubString aAddStr( aResId );
    ImplPrnDlgAddString( rStr, aAddStr );
}

// -----------------------------------------------------------------------

XubString ImplPrnDlgGetStatusText( const QueueInfo& rInfo )
{
    XubString   aStr;
    ULONG       nStatus = rInfo.GetStatus();

    // Default-Printer
    if ( rInfo.GetPrinterName().Len() &&
         (rInfo.GetPrinterName() == Printer::GetDefaultPrinterName()) )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_DEFPRINTER );

    // Status
    if ( nStatus & QUEUE_STATUS_READY )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_READY );
    if ( nStatus & QUEUE_STATUS_PAUSED )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAUSED );
    if ( nStatus & QUEUE_STATUS_PENDING_DELETION )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PENDING );
    if ( nStatus & QUEUE_STATUS_BUSY )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_BUSY );
    if ( nStatus & QUEUE_STATUS_INITIALIZING )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_INITIALIZING );
    if ( nStatus & QUEUE_STATUS_WAITING )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_WAITING );
    if ( nStatus & QUEUE_STATUS_WARMING_UP )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_WARMING_UP );
    if ( nStatus & QUEUE_STATUS_PROCESSING )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PROCESSING );
    if ( nStatus & QUEUE_STATUS_PRINTING )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PRINTING );
    if ( nStatus & QUEUE_STATUS_OFFLINE )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OFFLINE );
    if ( nStatus & QUEUE_STATUS_ERROR )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_ERROR );
    if ( nStatus & QUEUE_STATUS_SERVER_UNKNOWN )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_SERVER_UNKNOWN );
    if ( nStatus & QUEUE_STATUS_PAPER_JAM )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_JAM );
    if ( nStatus & QUEUE_STATUS_PAPER_OUT )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_OUT );
    if ( nStatus & QUEUE_STATUS_MANUAL_FEED )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_MANUAL_FEED );
    if ( nStatus & QUEUE_STATUS_PAPER_PROBLEM )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_PROBLEM );
    if ( nStatus & QUEUE_STATUS_IO_ACTIVE )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_IO_ACTIVE );
    if ( nStatus & QUEUE_STATUS_OUTPUT_BIN_FULL )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OUTPUT_BIN_FULL );
    if ( nStatus & QUEUE_STATUS_TONER_LOW )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_TONER_LOW );
    if ( nStatus & QUEUE_STATUS_NO_TONER )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_NO_TONER );
    if ( nStatus & QUEUE_STATUS_PAGE_PUNT )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAGE_PUNT );
    if ( nStatus & QUEUE_STATUS_USER_INTERVENTION )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_USER_INTERVENTION );
    if ( nStatus & QUEUE_STATUS_OUT_OF_MEMORY )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OUT_OF_MEMORY );
    if ( nStatus & QUEUE_STATUS_DOOR_OPEN )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_DOOR_OPEN );
    if ( nStatus & QUEUE_STATUS_POWER_SAVE )
        ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_POWER_SAVE );

    // Anzahl Jobs
    ULONG nJobs = rInfo.GetJobs();
    if ( nJobs && (nJobs != QUEUE_JOBS_DONTKNOW) )
    {
        XubString aJobStr( SvtResId( STR_SVT_PRNDLG_JOBCOUNT ) );
        XubString aJobs( XubString::CreateFromInt32( nJobs ) );
        aJobStr.SearchAndReplaceAscii( "%d", aJobs );
        ImplPrnDlgAddString( aStr, aJobStr );
    }

    return aStr;
}

// =======================================================================

PrinterSetupDialog::PrinterSetupDialog( Window* pWindow ) :
    ModalDialog     ( pWindow, SvtResId( DLG_SVT_PRNDLG_PRNSETUPDLG ) ),
    maGrpPrinter    ( this, SvtResId( GRP_PRINTER ) ),
    maFtName        ( this, SvtResId( FT_NAME ) ),
    maLbName        ( this, SvtResId( LB_NAMES ) ),
    maBtnProperties ( this, SvtResId( BTN_PROPERTIES ) ),
    maFtStatus      ( this, SvtResId( FT_STATUS ) ),
    maFiStatus      ( this, SvtResId( FI_STATUS ) ),
    maFtType        ( this, SvtResId( FT_TYPE ) ),
    maFiType        ( this, SvtResId( FI_TYPE ) ),
    maFtLocation    ( this, SvtResId( FT_LOCATION ) ),
    maFiLocation    ( this, SvtResId( FI_LOCATION ) ),
    maFtComment     ( this, SvtResId( FT_COMMENT ) ),
    maFiComment     ( this, SvtResId( FI_COMMENT ) ),
    maBtnOK         ( this, SvtResId( BTN_OK ) ),
    maBtnCancel     ( this, SvtResId( BTN_CANCEL ) ),
    maBtnHelp       ( this, SvtResId( BTN_HELP ) )
{
    FreeResource();

    mpPrinter       = NULL;
    mpTempPrinter   = NULL;

    maStatusTimer.SetTimeout( IMPL_PRINTDLG_STATUS_UPDATE );
    maStatusTimer.SetTimeoutHdl( LINK( this, PrinterSetupDialog, ImplStatusHdl ) );
    maBtnProperties.SetClickHdl( LINK( this, PrinterSetupDialog, ImplPropertiesHdl ) );
    maLbName.SetSelectHdl( LINK( this, PrinterSetupDialog, ImplChangePrinterHdl ) );
}

// -----------------------------------------------------------------------

PrinterSetupDialog::~PrinterSetupDialog()
{
    ImplFreePrnDlgListBox( &maLbName, FALSE );
    delete mpTempPrinter;
}

// -----------------------------------------------------------------------

void PrinterSetupDialog::ImplSetInfo()
{
    const QueueInfo* pInfo = (QueueInfo*)(maLbName.GetEntryData( maLbName.GetSelectEntryPos() ));
    if ( pInfo )
    {
        maFiType.SetText( pInfo->GetDriver() );
        maFiLocation.SetText( pInfo->GetLocation() );
        maFiComment.SetText( pInfo->GetComment() );
        maFiStatus.SetText( ImplPrnDlgGetStatusText( *pInfo ) );
    }
    else
    {
        XubString aTempStr;
        maFiType.SetText( aTempStr );
        maFiLocation.SetText( aTempStr );
        maFiComment.SetText( aTempStr );
        maFiStatus.SetText( aTempStr );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( PrinterSetupDialog, ImplStatusHdl, Timer*, EMPTYARG )
{
    QueueInfo aInfo;
    ImplPrnDlgUpdateQueueInfo( &maLbName, aInfo );
    maFiStatus.SetText( ImplPrnDlgGetStatusText( aInfo ) );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( PrinterSetupDialog, ImplPropertiesHdl, void*, EMPTYARG )
{
    if ( !mpTempPrinter )
        mpTempPrinter = new Printer( mpPrinter->GetJobSetup() );
    mpTempPrinter->Setup();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( PrinterSetupDialog, ImplChangePrinterHdl, void*, EMPTYARG )
{
    mpTempPrinter = ImplPrnDlgListBoxSelect( &maLbName, &maBtnProperties,
                                             mpPrinter, mpTempPrinter );
    ImplSetInfo();
    return 0;
}

// -----------------------------------------------------------------------

long PrinterSetupDialog::Notify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_GETFOCUS) && IsReallyVisible() )
        ImplStatusHdl( &maStatusTimer );

    return ModalDialog::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void PrinterSetupDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_PRINTER )
    {
        mpTempPrinter = ImplPrnDlgUpdatePrinter( mpPrinter, mpTempPrinter );
        Printer* pPrn;
        if ( mpTempPrinter )
            pPrn = mpTempPrinter;
        else
            pPrn = mpPrinter;
        ImplFillPrnDlgListBox( pPrn, &maLbName, &maBtnProperties );
        ImplSetInfo();
    }

    ModalDialog::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

short PrinterSetupDialog::Execute()
{
    if ( !mpPrinter || mpPrinter->IsPrinting() || mpPrinter->IsJobActive() )
    {
        DBG_ERRORFILE( "PrinterSetupDialog::Execute() - No Printer or printer is printing" );
        return FALSE;
    }

    ImplFillPrnDlgListBox( mpPrinter, &maLbName, &maBtnProperties );
    ImplSetInfo();
    maStatusTimer.Start();

    // Dialog starten
    short nRet = ModalDialog::Execute();

    // Wenn Dialog mit OK beendet wurde, dann die Daten updaten
    if ( nRet == TRUE )
    {
        if ( mpTempPrinter )
            mpPrinter->SetPrinterProps( mpTempPrinter );
    }

    maStatusTimer.Stop();

    return nRet;
}
