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

PrintMonitor::PrintMonitor(vcl::Window *pParent, PrintMonitorType eType )
    : CancelableModelessDialog(pParent, "PrintMonitorDialog",
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

CancelableModelessDialog::CancelableModelessDialog( vcl::Window *pParent,
        const OString& rID, const OUString& rUIXMLDescription )
    : ModelessDialog( pParent , rID, rUIXMLDescription )
{
    get(m_pCancelButton, "cancel");
}

void CancelableModelessDialog::SetCancelHdl( const Link& rLink )
{
    m_pCancelButton->SetClickHdl( rLink );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
