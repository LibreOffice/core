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
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <vcl/svapp.hxx>
#include <vcl/print.hxx>
#include <vcl/event.hxx>
#include <sal/log.hxx>

void ImplFillPrnDlgListBox( const Printer* pPrinter,
                            weld::ComboBox* pBox, weld::Button* pPropBtn )
{
    ImplFreePrnDlgListBox( pBox );

    const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
    unsigned int nCount = rPrinters.size();
    if ( nCount )
    {
        for( unsigned int i = 0; i < nCount; i++ )
            pBox->append_text( rPrinters[i] );
        pBox->set_active_text(pPrinter->GetName());
    }

    pBox->set_sensitive(nCount != 0);
    pPropBtn->set_visible( pPrinter->HasSupport( PrinterSupport::SetupDialog ) );
}


void ImplFreePrnDlgListBox( weld::ComboBox* pBox, bool bClear )
{
    if ( bClear )
        pBox->clear();
}


Printer* ImplPrnDlgListBoxSelect( const weld::ComboBox* pBox, weld::Button* pPropBtn,
                                  Printer const * pPrinter, Printer* pTempPrinterIn )
{
    VclPtr<Printer> pTempPrinter( pTempPrinterIn );
    if ( pBox->get_active() != -1 )
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( pBox->get_active_text(), true );
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

            pPropBtn->set_sensitive(pTempPrinter->HasSupport(PrinterSupport::SetupDialog));
        }
        else
            pPropBtn->set_sensitive(false);
    }
    else
        pPropBtn->set_sensitive(false);

    return pTempPrinter;
}


Printer* ImplPrnDlgUpdatePrinter( Printer const * pPrinter, Printer* pTempPrinterIn )
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


void ImplPrnDlgUpdateQueueInfo( const weld::ComboBox* pBox, QueueInfo& rInfo )
{
    if ( pBox->get_active() != -1 )
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( pBox->get_active_text(), true );
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


static OUString ImplPrnDlgAddResString(const OUString& rStr, const char* pResId)
{
    return ImplPrnDlgAddString(rStr, SvtResId(pResId));
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

    // Number of jobs
    sal_uInt32 nJobs = rInfo.GetJobs();
    if ( nJobs && (nJobs != QUEUE_JOBS_DONTKNOW) )
    {
        OUString aJobStr( SvtResId( STR_SVT_PRNDLG_JOBCOUNT ) );
        OUString aJobs( OUString::number( nJobs ) );
        aStr = ImplPrnDlgAddString(aStr, aJobStr.replaceAll("%d", aJobs));
    }

    return aStr;
}

PrinterSetupDialog::PrinterSetupDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "svt/ui/printersetupdialog.ui", "PrinterSetupDialog")
    , m_xLbName(m_xBuilder->weld_combo_box("name"))
    , m_xBtnProperties(m_xBuilder->weld_button("properties"))
    , m_xBtnOptions(m_xBuilder->weld_button("options"))
    , m_xFiStatus(m_xBuilder->weld_label("status"))
    , m_xFiType(m_xBuilder->weld_label("type"))
    , m_xFiLocation(m_xBuilder->weld_label("location"))
    , m_xFiComment(m_xBuilder->weld_label("comment"))
{
    m_xLbName->make_sorted();

    // show options button only if link is set
    m_xBtnOptions->hide();

    mpPrinter       = nullptr;
    mpTempPrinter   = nullptr;

    maStatusTimer.SetTimeout( IMPL_PRINTDLG_STATUS_UPDATE );
    maStatusTimer.SetInvokeHandler( LINK( this, PrinterSetupDialog, ImplStatusHdl ) );
    m_xBtnProperties->connect_clicked( LINK( this, PrinterSetupDialog, ImplPropertiesHdl ) );
    m_xLbName->connect_changed( LINK( this, PrinterSetupDialog, ImplChangePrinterHdl ) );
    m_xDialog->connect_focus_in( LINK( this, PrinterSetupDialog, ImplGetFocusHdl ) );
    Application::AddEventListener(LINK( this, PrinterSetupDialog, ImplDataChangedHdl ) );
}

PrinterSetupDialog::~PrinterSetupDialog()
{
    Application::RemoveEventListener(LINK( this, PrinterSetupDialog, ImplDataChangedHdl ) );
    ImplFreePrnDlgListBox(m_xLbName.get(), false);
}

void PrinterSetupDialog::SetOptionsHdl(const Link<weld::Button&, void>& rLink)
{
    m_xBtnOptions->connect_clicked(rLink);
    m_xBtnOptions->set_visible(rLink.IsSet());
}

void PrinterSetupDialog::ImplSetInfo()
{
    const QueueInfo* pInfo = Printer::GetQueueInfo(m_xLbName->get_active_text(), true);
    if ( pInfo )
    {
        m_xFiType->set_label( pInfo->GetDriver() );
        m_xFiLocation->set_label( pInfo->GetLocation() );
        m_xFiComment->set_label( pInfo->GetComment() );
        m_xFiStatus->set_label( ImplPrnDlgGetStatusText( *pInfo ) );
    }
    else
    {
        OUString aTempStr;
        m_xFiType->set_label( aTempStr );
        m_xFiLocation->set_label( aTempStr );
        m_xFiComment->set_label( aTempStr );
        m_xFiStatus->set_label( aTempStr );
    }
}

IMPL_LINK_NOARG(PrinterSetupDialog, ImplStatusHdl, Timer *, void)
{
    QueueInfo aInfo;
    ImplPrnDlgUpdateQueueInfo(m_xLbName.get(), aInfo);
    m_xFiStatus->set_label( ImplPrnDlgGetStatusText( aInfo ) );
}


IMPL_LINK_NOARG(PrinterSetupDialog, ImplPropertiesHdl, weld::Button&, void)
{
    if ( !mpTempPrinter )
        mpTempPrinter = VclPtr<Printer>::Create( mpPrinter->GetJobSetup() );
    mpTempPrinter->Setup(m_xDialog.get());
}

IMPL_LINK_NOARG(PrinterSetupDialog, ImplChangePrinterHdl, weld::ComboBox&, void)
{
    mpTempPrinter = ImplPrnDlgListBoxSelect(m_xLbName.get(), m_xBtnProperties.get(),
                                             mpPrinter, mpTempPrinter);
    ImplSetInfo();
}

IMPL_LINK(PrinterSetupDialog, ImplGetFocusHdl, weld::Widget&, rWidget, void)
{
    if (rWidget.is_visible())
        ImplStatusHdl(&maStatusTimer);
}

IMPL_LINK(PrinterSetupDialog, ImplDataChangedHdl, VclSimpleEvent&, rEvt, void)
{
    VclEventId nEvent = rEvt.GetId();
    if (nEvent != VclEventId::ApplicationDataChanged)
        return;

    DataChangedEvent* pData = static_cast<DataChangedEvent*>(static_cast<VclWindowEvent&>(rEvt).GetData());
    if (!pData || pData->GetType() != DataChangedEventType::PRINTER)
        return;

    mpTempPrinter = ImplPrnDlgUpdatePrinter(mpPrinter, mpTempPrinter);
    Printer* pPrn;
    if (mpTempPrinter)
        pPrn = mpTempPrinter;
    else
        pPrn = mpPrinter;
    ImplFillPrnDlgListBox(pPrn, m_xLbName.get(), m_xBtnProperties.get());
    ImplSetInfo();
}

short PrinterSetupDialog::run()
{
    if ( !mpPrinter || mpPrinter->IsPrinting() || mpPrinter->IsJobActive() )
    {
        SAL_WARN( "svtools.dialogs", "PrinterSetupDialog::execute() - No Printer or printer is printing" );
        return RET_CANCEL;
    }

    Printer::updatePrinters();

    ImplFillPrnDlgListBox(mpPrinter, m_xLbName.get(), m_xBtnProperties.get());
    ImplSetInfo();
    maStatusTimer.Start();

    // start dialog
    short nRet = GenericDialogController::run();

    // update data if the dialog was terminated with OK
    if ( nRet == RET_OK && mpTempPrinter )
        mpPrinter->SetPrinterProps( mpTempPrinter );

    maStatusTimer.Stop();

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
