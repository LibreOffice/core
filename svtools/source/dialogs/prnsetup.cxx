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
                                  Printer* pPrinter, Printer* pTempPrinterIn )
{
    VclPtr<Printer> pTempPrinter( pTempPrinterIn );
    if ( pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( pBox->GetSelectEntry(), true );
        if( pInfo)
        {
            if ( !pTempPrinter )
            {
                if ( (pPrinter->GetName() == pInfo->GetPrinterName()) &&
                     (pPrinter->GetDriverName() == pInfo->GetDriver()) )
                    pTempPrinter = VclPtr<Printer>::Create( pPrinter->GetJobSetup() );
                else
                    pTempPrinter = VclPtr<Printer>::Create( *pInfo );
            }
            else
            {
                if ( (pTempPrinter->GetName() != pInfo->GetPrinterName()) ||
                     (pTempPrinter->GetDriverName() != pInfo->GetDriver()) )
                {
                    pTempPrinter.disposeAndClear();
                    pTempPrinter = VclPtr<Printer>::Create( *pInfo );
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



Printer* ImplPrnDlgUpdatePrinter( Printer* pPrinter, Printer* pTempPrinterIn )
{
    VclPtr<Printer> pTempPrinter( pTempPrinterIn );
    OUString aPrnName;
    if ( pTempPrinter )
        aPrnName = pTempPrinter->GetName();
    else
        aPrnName = pPrinter->GetName();

    if ( ! Printer::GetQueueInfo( aPrnName, false ) )
    {
        pTempPrinter.disposeAndClear();
        pTempPrinter = VclPtr<Printer>::Create();
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
    PrintQueueFlags nStatus = rInfo.GetStatus();

    // Default-Printer
    if ( !rInfo.GetPrinterName().isEmpty() &&
         (rInfo.GetPrinterName() == Printer::GetDefaultPrinterName()) )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_DEFPRINTER );

    // Status
    if ( nStatus & PrintQueueFlags::Ready )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_READY );
    if ( nStatus & PrintQueueFlags::Paused )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAUSED );
    if ( nStatus & PrintQueueFlags::PendingDeletion )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PENDING );
    if ( nStatus & PrintQueueFlags::Busy )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_BUSY );
    if ( nStatus & PrintQueueFlags::Initializing )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_INITIALIZING );
    if ( nStatus & PrintQueueFlags::Waiting )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_WAITING );
    if ( nStatus & PrintQueueFlags::WarmingUp )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_WARMING_UP );
    if ( nStatus & PrintQueueFlags::Processing )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PROCESSING );
    if ( nStatus & PrintQueueFlags::Printing )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PRINTING );
    if ( nStatus & PrintQueueFlags::Offline )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OFFLINE );
    if ( nStatus & PrintQueueFlags::Error )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_ERROR );
    if ( nStatus & PrintQueueFlags::StatusUnknown )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_SERVER_UNKNOWN );
    if ( nStatus & PrintQueueFlags::PaperJam )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_JAM );
    if ( nStatus & PrintQueueFlags::PaperOut )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_OUT );
    if ( nStatus & PrintQueueFlags::ManualFeed )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_MANUAL_FEED );
    if ( nStatus & PrintQueueFlags::PaperProblem )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAPER_PROBLEM );
    if ( nStatus & PrintQueueFlags::IOActive )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_IO_ACTIVE );
    if ( nStatus & PrintQueueFlags::OutputBinFull )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OUTPUT_BIN_FULL );
    if ( nStatus & PrintQueueFlags::TonerLow )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_TONER_LOW );
    if ( nStatus & PrintQueueFlags::NoToner )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_NO_TONER );
    if ( nStatus & PrintQueueFlags::PagePunt )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_PAGE_PUNT );
    if ( nStatus & PrintQueueFlags::UserIntervention )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_USER_INTERVENTION );
    if ( nStatus & PrintQueueFlags::OutOfMemory )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_OUT_OF_MEMORY );
    if ( nStatus & PrintQueueFlags::DoorOpen )
        aStr = ImplPrnDlgAddResString( aStr, STR_SVT_PRNDLG_DOOR_OPEN );
    if ( nStatus & PrintQueueFlags::PowerSave )
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

    mpPrinter       = nullptr;
    mpTempPrinter   = nullptr;

    maStatusTimer.SetTimeout( IMPL_PRINTDLG_STATUS_UPDATE );
    maStatusTimer.SetTimeoutHdl( LINK( this, PrinterSetupDialog, ImplStatusHdl ) );
    m_pBtnProperties->SetClickHdl( LINK( this, PrinterSetupDialog, ImplPropertiesHdl ) );
    m_pLbName->SetSelectHdl( LINK( this, PrinterSetupDialog, ImplChangePrinterHdl ) );
}



PrinterSetupDialog::~PrinterSetupDialog()
{
    disposeOnce();
}

void PrinterSetupDialog::dispose()
{
    ImplFreePrnDlgListBox(m_pLbName, false);
    m_pLbName.clear();
    m_pBtnProperties.clear();
    m_pBtnOptions.clear();
    m_pFiStatus.clear();
    m_pFiType.clear();
    m_pFiLocation.clear();
    m_pFiComment.clear();
    mpTempPrinter.disposeAndClear();
    mpPrinter.clear();
    ModalDialog::dispose();
}

void PrinterSetupDialog::SetOptionsHdl( const Link<Button*,void>& rLink )
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



IMPL_LINK_NOARG_TYPED(PrinterSetupDialog, ImplStatusHdl, Timer *, void)
{
    QueueInfo aInfo;
    ImplPrnDlgUpdateQueueInfo(m_pLbName, aInfo);
    m_pFiStatus->SetText( ImplPrnDlgGetStatusText( aInfo ) );
}



IMPL_LINK_NOARG_TYPED(PrinterSetupDialog, ImplPropertiesHdl, Button*, void)
{
    if ( !mpTempPrinter )
        mpTempPrinter = VclPtr<Printer>::Create( mpPrinter->GetJobSetup() );
    mpTempPrinter->Setup( this );
}



IMPL_LINK_NOARG_TYPED(PrinterSetupDialog, ImplChangePrinterHdl, ListBox&, void)
{
    mpTempPrinter = ImplPrnDlgListBoxSelect(m_pLbName, m_pBtnProperties,
                                             mpPrinter, mpTempPrinter );
    ImplSetInfo();
}



bool PrinterSetupDialog::Notify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS) && IsReallyVisible() )
        ImplStatusHdl( &maStatusTimer );

    return ModalDialog::Notify( rNEvt );
}



void PrinterSetupDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DataChangedEventType::PRINTER )
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
        return RET_CANCEL;
    }

    Printer::updatePrinters();

    ImplFillPrnDlgListBox(mpPrinter, m_pLbName, m_pBtnProperties);
    ImplSetInfo();
    maStatusTimer.Start();

    // start dialog
    short nRet = ModalDialog::Execute();

    // update data if the dialog was terminated with OK
    if ( nRet == RET_OK )
    {
        if ( mpTempPrinter )
            mpPrinter->SetPrinterProps( mpTempPrinter );
    }

    maStatusTimer.Stop();

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
