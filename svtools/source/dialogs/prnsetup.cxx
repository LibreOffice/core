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

#include <svtools/prnsetup.hxx>
#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>
#include <vcl/print.hxx>



void ImplFillPrnDlgListBox( const Printer* pPrinter,
                            ListBox* pBox, PushButton* pPropBtn )
{
    ImplFreePrnDlgListBox( pBox );

    const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
    unsigned int nCount = rPrinters.size();
    if ( nCount )
    {
        for( unsigned int i = 0; i < nCount; i++ )
            pBox->InsertEntry( rPrinters[i] );
        pBox->SelectEntry( pPrinter->GetName() );
    }

    pBox->Enable( nCount != 0 );
    pPropBtn->Show( pPrinter->HasSupport( SUPPORT_SETUPDIALOG ) );
}



void ImplFreePrnDlgListBox( ListBox* pBox, bool bClear )
{
    if ( bClear )
        pBox->Clear();
}



Printer* ImplPrnDlgListBoxSelect( ListBox* pBox, PushButton* pPropBtn,
                                  Printer* pPrinter, Printer* pTempPrinter )
{
    if ( pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( pBox->GetSelectEntry(), true );
        if( pInfo)
        {
            if ( !pTempPrinter )
            {
                if ( (pPrinter->GetName() == pInfo->GetPrinterName()) &&
                     (pPrinter->GetDriverName() == pInfo->GetDriver()) )
                    pTempPrinter = new Printer( pPrinter->GetJobSetup() );
                else
                    pTempPrinter = new Printer( *pInfo );
            }
            else
            {
                if ( (pTempPrinter->GetName() != pInfo->GetPrinterName()) ||
                     (pTempPrinter->GetDriverName() != pInfo->GetDriver()) )
                {
                    delete pTempPrinter;
                    pTempPrinter = new Printer( *pInfo );
                }
            }

            pPropBtn->Enable( pTempPrinter->HasSupport( SUPPORT_SETUPDIALOG ) );
        }
        else
            pPropBtn->Disable();
    }
    else
        pPropBtn->Disable();

    return pTempPrinter;
}



Printer* ImplPrnDlgUpdatePrinter( Printer* pPrinter, Printer* pTempPrinter )
{
    OUString aPrnName;
    if ( pTempPrinter )
        aPrnName = pTempPrinter->GetName();
    else
        aPrnName = pPrinter->GetName();

    if ( ! Printer::GetQueueInfo( aPrnName, false ) )
    {
        if ( pTempPrinter )
            delete pTempPrinter;
        pTempPrinter = new Printer;
    }

    return pTempPrinter;
}



void ImplPrnDlgUpdateQueueInfo( ListBox* pBox, QueueInfo& rInfo )
{
    if ( pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( pBox->GetSelectEntry(), true );
        if( pInfo )
            rInfo = *pInfo;
    }
}



static OUString ImplPrnDlgAddString(const OUString& rStr, const OUString& rAddStr)
{
    OUString aStr(rStr);
    if (!aStr.isEmpty())
        aStr += "; " ;
    return aStr + rAddStr;
}



static OUString ImplPrnDlgAddResString(const OUString& rStr, sal_uInt16 nResId)
{
    return ImplPrnDlgAddString(rStr, SVT_RESSTR(nResId));
}



OUString ImplPrnDlgGetStatusText( const QueueInfo& rInfo )
{
    OUString aStr;
    sal_uLong nStatus = rInfo.GetStatus();

    // Default-Printer
    if ( !rInfo.GetPrinterName().isEmpty() &&
         (rInfo.GetPrinterName() == Printer::GetDefaultPrinterName()) )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_DEFPRINTER );

    // Status
    if ( nStatus & QUEUE_STATUS_READY )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_READY );
    if ( nStatus & QUEUE_STATUS_PAUSED )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAUSED );
    if ( nStatus & QUEUE_STATUS_PENDING_DELETION )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PENDING );
    if ( nStatus & QUEUE_STATUS_BUSY )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_BUSY );
    if ( nStatus & QUEUE_STATUS_INITIALIZING )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_INITIALIZING );
    if ( nStatus & QUEUE_STATUS_WAITING )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_WAITING );
    if ( nStatus & QUEUE_STATUS_WARMING_UP )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_WARMING_UP );
    if ( nStatus & QUEUE_STATUS_PROCESSING )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PROCESSING );
    if ( nStatus & QUEUE_STATUS_PRINTING )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PRINTING );
    if ( nStatus & QUEUE_STATUS_OFFLINE )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OFFLINE );
    if ( nStatus & QUEUE_STATUS_ERROR )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_ERROR );
    if ( nStatus & QUEUE_STATUS_SERVER_UNKNOWN )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_SERVER_UNKNOWN );
    if ( nStatus & QUEUE_STATUS_PAPER_JAM )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_JAM );
    if ( nStatus & QUEUE_STATUS_PAPER_OUT )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_OUT );
    if ( nStatus & QUEUE_STATUS_MANUAL_FEED )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_MANUAL_FEED );
    if ( nStatus & QUEUE_STATUS_PAPER_PROBLEM )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_PROBLEM );
    if ( nStatus & QUEUE_STATUS_IO_ACTIVE )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_IO_ACTIVE );
    if ( nStatus & QUEUE_STATUS_OUTPUT_BIN_FULL )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OUTPUT_BIN_FULL );
    if ( nStatus & QUEUE_STATUS_TONER_LOW )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_TONER_LOW );
    if ( nStatus & QUEUE_STATUS_NO_TONER )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_NO_TONER );
    if ( nStatus & QUEUE_STATUS_PAGE_PUNT )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAGE_PUNT );
    if ( nStatus & QUEUE_STATUS_USER_INTERVENTION )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_USER_INTERVENTION );
    if ( nStatus & QUEUE_STATUS_OUT_OF_MEMORY )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OUT_OF_MEMORY );
    if ( nStatus & QUEUE_STATUS_DOOR_OPEN )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_DOOR_OPEN );
    if ( nStatus & QUEUE_STATUS_POWER_SAVE )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_POWER_SAVE );

    // Anzahl Jobs
    sal_uLong nJobs = rInfo.GetJobs();
    if ( nJobs && (nJobs != QUEUE_JOBS_DONTKNOW) )
    {
        OUString aJobStr( SVT_RESSTR( STR_SVT_PRNDLG_JOBCOUNT ) );
        OUString aJobs( OUString::number( nJobs ) );
        aStr = ImplPrnDlgAddString(aStr, aJobStr.replaceAll("%d", aJobs));
    }

    return aStr;
}



PrinterSetupDialog::PrinterSetupDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "PrinterSetupDialog",
        "svt/ui/printersetupdialog.ui")
{
    get(m_pLbName, "name");
    m_pLbName->SetStyle(m_pLbName->GetStyle() | WB_SORT);
    get(m_pBtnProperties, "properties");
    get(m_pBtnOptions, "options");
    get(m_pFiStatus, "status");
    get(m_pFiType, "type");
    get(m_pFiLocation, "location");
    get(m_pFiComment, "comment");

    // show options button only if link is set
    m_pBtnOptions->Hide();

    mpPrinter       = NULL;
    mpTempPrinter   = NULL;

    maStatusTimer.SetTimeout( IMPL_PRINTDLG_STATUS_UPDATE );
    maStatusTimer.SetTimeoutHdl( LINK( this, PrinterSetupDialog, ImplStatusHdl ) );
    m_pBtnProperties->SetClickHdl( LINK( this, PrinterSetupDialog, ImplPropertiesHdl ) );
    m_pLbName->SetSelectHdl( LINK( this, PrinterSetupDialog, ImplChangePrinterHdl ) );
}



PrinterSetupDialog::~PrinterSetupDialog()
{
    ImplFreePrnDlgListBox(m_pLbName, false);
    delete mpTempPrinter;
}



void PrinterSetupDialog::SetOptionsHdl( const Link& rLink )
{
    m_pBtnOptions->SetClickHdl( rLink );
    m_pBtnOptions->Show( rLink.IsSet() );
}

void PrinterSetupDialog::ImplSetInfo()
{
    const QueueInfo* pInfo = Printer::GetQueueInfo(m_pLbName->GetSelectEntry(), true);
    if ( pInfo )
    {
        m_pFiType->SetText( pInfo->GetDriver() );
        m_pFiLocation->SetText( pInfo->GetLocation() );
        m_pFiComment->SetText( pInfo->GetComment() );
        m_pFiStatus->SetText( ImplPrnDlgGetStatusText( *pInfo ) );
    }
    else
    {
        OUString aTempStr;
        m_pFiType->SetText( aTempStr );
        m_pFiLocation->SetText( aTempStr );
        m_pFiComment->SetText( aTempStr );
        m_pFiStatus->SetText( aTempStr );
    }
}



IMPL_LINK_NOARG(PrinterSetupDialog, ImplStatusHdl)
{
    QueueInfo aInfo;
    ImplPrnDlgUpdateQueueInfo(m_pLbName, aInfo);
    m_pFiStatus->SetText( ImplPrnDlgGetStatusText( aInfo ) );

    return 0;
}



IMPL_LINK_NOARG(PrinterSetupDialog, ImplPropertiesHdl)
{
    if ( !mpTempPrinter )
        mpTempPrinter = new Printer( mpPrinter->GetJobSetup() );
    mpTempPrinter->Setup( this );

    return 0;
}



IMPL_LINK_NOARG(PrinterSetupDialog, ImplChangePrinterHdl)
{
    mpTempPrinter = ImplPrnDlgListBoxSelect(m_pLbName, m_pBtnProperties,
                                             mpPrinter, mpTempPrinter );
    ImplSetInfo();
    return 0;
}



bool PrinterSetupDialog::Notify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_GETFOCUS) && IsReallyVisible() )
        ImplStatusHdl( &maStatusTimer );

    return ModalDialog::Notify( rNEvt );
}



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
        ImplFillPrnDlgListBox(pPrn, m_pLbName, m_pBtnProperties);
        ImplSetInfo();
    }

    ModalDialog::DataChanged( rDCEvt );
}



short PrinterSetupDialog::Execute()
{
    if ( !mpPrinter || mpPrinter->IsPrinting() || mpPrinter->IsJobActive() )
    {
        SAL_WARN( "svtools.dialogs", "PrinterSetupDialog::Execute() - No Printer or printer is printing" );
        return sal_False;
    }

    Printer::updatePrinters();

    ImplFillPrnDlgListBox(mpPrinter, m_pLbName, m_pBtnProperties);
    ImplSetInfo();
    maStatusTimer.Start();

    // start dialog
    short nRet = ModalDialog::Execute();

    // update data if the dialog was terminated with OK
    if ( nRet == sal_True )
    {
        if ( mpTempPrinter )
            mpPrinter->SetPrinterProps( mpTempPrinter );
    }

    maStatusTimer.Stop();

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
