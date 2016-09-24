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

#include "wrtsh.hxx"

#include "dbui.hrc"
#include "dbui.hxx"

PrintMonitor::PrintMonitor(vcl::Window *pParent, bool modal, PrintMonitorType eType )
    : CancelableDialog(pParent, modal, "PrintMonitorDialog",
        "modules/swriter/ui/printmonitordialog.ui")
{
    get(m_pDocName, "docname");
    get(m_pPrinter, "printer");
    get(m_pPrintInfo, "printinfo");
    switch (eType)
    {
        case MONITOR_TYPE_SAVE:
            SetText(get<FixedText>("alttitle")->GetText());
            get(m_pPrinting, "saving");
            break;
        case MONITOR_TYPE_PRINT:
            get(m_pPrinting, "printing");
            break;
    }
    m_pPrinting->Show();
}

PrintMonitor::~PrintMonitor()
{
    disposeOnce();
}

void PrintMonitor::dispose()
{
    m_pDocName.clear();
    m_pPrinting.clear();
    m_pPrinter.clear();
    m_pPrintInfo.clear();

    CancelableDialog::dispose();
}

// Progress Indicator for Creation of personalized Mail Merge documents:
CreateMonitor::CreateMonitor( vcl::Window *pParent, bool modal )
    : CancelableDialog(pParent, modal, "MMCreatingDialog",
        "modules/swriter/ui/mmcreatingdialog.ui")
    , m_sCountingPattern()
    , m_sVariable_Total("%Y")
    , m_sVariable_Position("%X")
    , m_nTotalCount(0)
    , m_nCurrentPosition(0)
{
    get(m_pCounting, "progress");
    m_sCountingPattern = m_pCounting->GetText();
    m_pCounting->SetText("...");
}

CreateMonitor::~CreateMonitor()
{
    disposeOnce();
}

void CreateMonitor::dispose()
{
    m_pCancelButton.clear();
    m_pCounting.clear();

    CancelableDialog::dispose();
}

void CreateMonitor::UpdateCountingText()
{
    OUString sText(m_sCountingPattern);
    sText = sText.replaceAll( m_sVariable_Total, OUString::number( m_nTotalCount ) );
    sText = sText.replaceAll( m_sVariable_Position, OUString::number( m_nCurrentPosition ) );
    m_pCounting->SetText(sText);
}

void CreateMonitor::SetTotalCount( sal_Int32 nTotal )
{
    m_nTotalCount = nTotal;
    UpdateCountingText();
}

void CreateMonitor::SetCurrentPosition( sal_Int32 nCurrent )
{
    m_nCurrentPosition = nCurrent;
    UpdateCountingText();
}

CancelableDialog::CancelableDialog( vcl::Window *pParent, bool modal,
        const OUString& rID, const OUString& rUIXMLDescription )
    : Dialog( pParent , rID, rUIXMLDescription,
              modal ? WINDOW_MODALDIALOG : WINDOW_MODELESSDIALOG )
    , mbModal( modal )
{
    get(m_pCancelButton, "cancel");
}

CancelableDialog::~CancelableDialog()
{
    disposeOnce();
}

void CancelableDialog::dispose()
{
    EndDialog();
    m_pCancelButton.clear();

    Dialog::dispose();
}

void CancelableDialog::SetCancelHdl( const Link<Button*,void>& rLink )
{
    m_pCancelButton->SetClickHdl( rLink );
}

void CancelableDialog::Show()
{
    if (mbModal)
        StartExecuteModal( Link<Dialog&,void>() );
    else
        Dialog::Show();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
