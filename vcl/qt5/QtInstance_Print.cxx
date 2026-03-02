/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <string_view>

#include <QtInstance.hxx>

#include <printerinfomanager.hxx>

#include <print.h>

#include <unx/genpspgraphics.h>

using namespace psp;

SalInfoPrinter* QtInstance::CreateInfoPrinter(SalPrinterQueueInfo* pQueueInfo,
                                              ImplJobSetup* pJobSetup)
{
    // create and initialize SalInfoPrinter
    PspSalInfoPrinter* pPrinter = new PspSalInfoPrinter;
    configurePspInfoPrinter(pPrinter, pQueueInfo, pJobSetup);

    return pPrinter;
}

std::unique_ptr<SalPrinter> QtInstance::CreatePrinter(SalInfoPrinter* pInfoPrinter)
{
    // create and initialize SalPrinter
    PspSalPrinter* pPrinter = new PspSalPrinter(pInfoPrinter);
    pPrinter->m_aJobData = static_cast<PspSalInfoPrinter*>(pInfoPrinter)->m_aJobData;

    return std::unique_ptr<SalPrinter>(pPrinter);
}

void QtInstance::GetPrinterQueueInfo(ImplPrnQueueList* pList)
{
    PrinterInfoManager& rManager(PrinterInfoManager::get());
    static const char* pNoSyncDetection = getenv("SAL_DISABLE_SYNCHRONOUS_PRINTER_DETECTION");
    if (!pNoSyncDetection || !*pNoSyncDetection)
    {
        // #i62663# synchronize possible asynchronouse printer detection now
        rManager.checkPrintersChanged(true);
    }
    ::std::vector<OUString> aPrinters;
    rManager.listPrinters(aPrinters);

    for (const auto& rPrinter : aPrinters)
    {
        const PrinterInfo& rInfo(rManager.getPrinterInfo(rPrinter));
        // create new entry
        std::unique_ptr<SalPrinterQueueInfo> pInfo(new SalPrinterQueueInfo);
        pInfo->maPrinterName = rPrinter;
        pInfo->maDriver = rInfo.m_aDriverName;
        pInfo->maLocation = rInfo.m_aLocation;
        pInfo->maComment = rInfo.m_aComment;

        OUString sPdfDir;
        if (getPdfDir(rInfo, sPdfDir))
            pInfo->maLocation = sPdfDir;

        pList->Add(std::move(pInfo));
    }
}

void QtInstance::GetPrinterQueueState(SalPrinterQueueInfo*) {}

OUString QtInstance::GetDefaultPrinter()
{
    PrinterInfoManager& rManager(PrinterInfoManager::get());
    return rManager.getDefaultPrinter();
}

void QtInstance::PostPrintersChanged() {}

std::unique_ptr<GenPspGraphics> QtInstance::CreatePrintGraphics()
{
    return std::make_unique<GenPspGraphics>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
