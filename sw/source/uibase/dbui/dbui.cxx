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

#include <dbui.hxx>

SaveMonitor::SaveMonitor(weld::Window *pParent)
    : GenericDialogController(pParent, "modules/swriter/ui/savemonitordialog.ui",
        "SaveMonitorDialog")
    , m_xDocName(m_xBuilder->weld_label("docname"))
    , m_xPrinter(m_xBuilder->weld_label("printer"))
    , m_xPrintInfo(m_xBuilder->weld_label("printinfo"))
{
}

SaveMonitor::~SaveMonitor()
{
}

PrintMonitor::PrintMonitor(weld::Window *pParent)
    : GenericDialogController(pParent, "modules/swriter/ui/printmonitordialog.ui",
        "PrintMonitorDialog")
    , m_xDocName(m_xBuilder->weld_label("docname"))
    , m_xPrinter(m_xBuilder->weld_label("printer"))
    , m_xPrintInfo(m_xBuilder->weld_label("printinfo"))
{
}

PrintMonitor::~PrintMonitor()
{
}

// Progress Indicator for Creation of personalized Mail Merge documents:
CreateMonitor::CreateMonitor(weld::Window *pParent)
    : GenericDialogController(pParent, "modules/swriter/ui/mmcreatingdialog.ui",
        "MMCreatingDialog")
    , m_sCountingPattern()
    , m_nTotalCount(0)
    , m_nCurrentPosition(0)
    , m_xCounting(m_xBuilder->weld_label("progress"))
{
    m_sCountingPattern = m_xCounting->get_label();
    m_xCounting->set_label("...");
}

CreateMonitor::~CreateMonitor()
{
}

void CreateMonitor::UpdateCountingText()
{
    constexpr OUStringLiteral sVariable_Total(u"%Y");
    constexpr OUStringLiteral sVariable_Position(u"%X");

    OUString sText(m_sCountingPattern);
    sText = sText.replaceAll( sVariable_Total, OUString::number( m_nTotalCount ) );
    sText = sText.replaceAll( sVariable_Position, OUString::number( m_nCurrentPosition ) );
    m_xCounting->set_label(sText);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
