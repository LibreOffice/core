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

#include <jobset.h>
#include <print.h>
#include <win/WindowsInstance.hxx>
#include <win/salprn.h>

#include <comphelper/solarmutex.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <winspool.h>
#if defined GetDefaultPrinter
#undef GetDefaultPrinter
#endif

static PrintQueueFlags ImplWinQueueStatusToSal(DWORD nWinStatus)
{
    PrintQueueFlags nStatus = PrintQueueFlags::NONE;
    if (nWinStatus & PRINTER_STATUS_PAUSED)
        nStatus |= PrintQueueFlags::Paused;
    if (nWinStatus & PRINTER_STATUS_ERROR)
        nStatus |= PrintQueueFlags::Error;
    if (nWinStatus & PRINTER_STATUS_PENDING_DELETION)
        nStatus |= PrintQueueFlags::PendingDeletion;
    if (nWinStatus & PRINTER_STATUS_PAPER_JAM)
        nStatus |= PrintQueueFlags::PaperJam;
    if (nWinStatus & PRINTER_STATUS_PAPER_OUT)
        nStatus |= PrintQueueFlags::PaperOut;
    if (nWinStatus & PRINTER_STATUS_MANUAL_FEED)
        nStatus |= PrintQueueFlags::ManualFeed;
    if (nWinStatus & PRINTER_STATUS_PAPER_PROBLEM)
        nStatus |= PrintQueueFlags::PaperProblem;
    if (nWinStatus & PRINTER_STATUS_OFFLINE)
        nStatus |= PrintQueueFlags::Offline;
    if (nWinStatus & PRINTER_STATUS_IO_ACTIVE)
        nStatus |= PrintQueueFlags::IOActive;
    if (nWinStatus & PRINTER_STATUS_BUSY)
        nStatus |= PrintQueueFlags::Busy;
    if (nWinStatus & PRINTER_STATUS_PRINTING)
        nStatus |= PrintQueueFlags::Printing;
    if (nWinStatus & PRINTER_STATUS_OUTPUT_BIN_FULL)
        nStatus |= PrintQueueFlags::OutputBinFull;
    if (nWinStatus & PRINTER_STATUS_WAITING)
        nStatus |= PrintQueueFlags::Waiting;
    if (nWinStatus & PRINTER_STATUS_PROCESSING)
        nStatus |= PrintQueueFlags::Processing;
    if (nWinStatus & PRINTER_STATUS_INITIALIZING)
        nStatus |= PrintQueueFlags::Initializing;
    if (nWinStatus & PRINTER_STATUS_WARMING_UP)
        nStatus |= PrintQueueFlags::WarmingUp;
    if (nWinStatus & PRINTER_STATUS_TONER_LOW)
        nStatus |= PrintQueueFlags::TonerLow;
    if (nWinStatus & PRINTER_STATUS_NO_TONER)
        nStatus |= PrintQueueFlags::NoToner;
    if (nWinStatus & PRINTER_STATUS_PAGE_PUNT)
        nStatus |= PrintQueueFlags::PagePunt;
    if (nWinStatus & PRINTER_STATUS_USER_INTERVENTION)
        nStatus |= PrintQueueFlags::UserIntervention;
    if (nWinStatus & PRINTER_STATUS_OUT_OF_MEMORY)
        nStatus |= PrintQueueFlags::OutOfMemory;
    if (nWinStatus & PRINTER_STATUS_DOOR_OPEN)
        nStatus |= PrintQueueFlags::DoorOpen;
    if (nWinStatus & PRINTER_STATUS_SERVER_UNKNOWN)
        nStatus |= PrintQueueFlags::StatusUnknown;
    if (nWinStatus & PRINTER_STATUS_POWER_SAVE)
        nStatus |= PrintQueueFlags::PowerSave;
    if (nStatus == PrintQueueFlags::NONE && !(nWinStatus & PRINTER_STATUS_NOT_AVAILABLE))
        nStatus |= PrintQueueFlags::Ready;
    return nStatus;
}

WindowsInstance::WindowsInstance(std::unique_ptr<comphelper::SolarMutex> pMutex, SalData* pSalData,
                                 const OUString& rToolkitName)
    : SalInstance(std::move(pMutex), pSalData, rToolkitName)
{
}

WindowsInstance::~WindowsInstance() {}

SalInfoPrinter* WindowsInstance::CreateInfoPrinter(SalPrinterQueueInfo* pQueueInfo,
                                                   ImplJobSetup* pSetupData)
{
    WinSalInfoPrinter* pPrinter = new WinSalInfoPrinter;
    if (!pQueueInfo->moPortName)
        GetPrinterQueueState(pQueueInfo);
    pPrinter->maDriverName = pQueueInfo->maDriver;
    pPrinter->maDeviceName = pQueueInfo->maPrinterName;
    pPrinter->maPortName = pQueueInfo->moPortName ? *pQueueInfo->moPortName : OUString();

    // check if the provided setup data match the actual printer
    ImplTestSalJobSetup(pPrinter, pSetupData, true);

    HDC hDC = ImplCreateSalPrnIC(pPrinter, pSetupData);
    if (!hDC)
    {
        delete pPrinter;
        return nullptr;
    }

    pPrinter->setHDC(hDC);
    if (!pSetupData->GetDriverData())
        ImplUpdateSalJobSetup(pPrinter, pSetupData, false, nullptr);
    ImplDevModeToJobSetup(pPrinter, pSetupData, JobSetFlags::ALL);
    pSetupData->SetSystem(JOBSETUP_SYSTEM_WINDOWS);

    return pPrinter;
}

std::unique_ptr<SalPrinter> WindowsInstance::CreatePrinter(SalInfoPrinter* pInfoPrinter)
{
    WinSalPrinter* pPrinter = new WinSalPrinter;
    pPrinter->mpInfoPrinter = static_cast<WinSalInfoPrinter*>(pInfoPrinter);
    return std::unique_ptr<SalPrinter>(pPrinter);
}

void WindowsInstance::GetPrinterQueueInfo(ImplPrnQueueList* pList)
{
    DWORD i;
    DWORD nBytes = 0;
    DWORD nInfoPrn4 = 0;
    EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, nullptr, 0, &nBytes,
                  &nInfoPrn4);
    if (!nBytes)
        return;

    PRINTER_INFO_4W* pWinInfo4 = static_cast<PRINTER_INFO_4W*>(std::malloc(nBytes));
    assert(pWinInfo4 && "Don't handle OOM conditions");
    if (EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4,
                      reinterpret_cast<LPBYTE>(pWinInfo4), nBytes, &nBytes, &nInfoPrn4))
    {
        for (i = 0; i < nInfoPrn4; i++)
        {
            std::unique_ptr<SalPrinterQueueInfo> pInfo(new SalPrinterQueueInfo);
            pInfo->maPrinterName = o3tl::toU(pWinInfo4[i].pPrinterName);
            pInfo->mnStatus = PrintQueueFlags::NONE;
            pInfo->mnJobs = 0;
            pList->Add(std::move(pInfo));
        }
    }
    std::free(pWinInfo4);
}

void WindowsInstance::GetPrinterQueueState(SalPrinterQueueInfo* pInfo)
{
    HANDLE hPrinter = nullptr;
    LPWSTR pPrnName = const_cast<LPWSTR>(o3tl::toW(pInfo->maPrinterName.getStr()));
    if (!OpenPrinterW(pPrnName, &hPrinter, nullptr))
        return;

    DWORD nBytes = 0;
    GetPrinterW(hPrinter, 2, nullptr, 0, &nBytes);
    if (nBytes)
    {
        PRINTER_INFO_2W* pWinInfo2 = static_cast<PRINTER_INFO_2W*>(std::malloc(nBytes));
        assert(pWinInfo2 && "Don't handle OOM conditions");
        if (GetPrinterW(hPrinter, 2, reinterpret_cast<LPBYTE>(pWinInfo2), nBytes, &nBytes))
        {
            if (pWinInfo2->pDriverName)
                pInfo->maDriver = o3tl::toU(pWinInfo2->pDriverName);
            OUString aPortName;
            if (pWinInfo2->pPortName)
                aPortName = o3tl::toU(pWinInfo2->pPortName);
            // pLocation can be 0 (the Windows docu doesn't describe this)
            if (pWinInfo2->pLocation && *pWinInfo2->pLocation)
                pInfo->maLocation = o3tl::toU(pWinInfo2->pLocation);
            else
                pInfo->maLocation = aPortName;
            // pComment can be 0 (the Windows docu doesn't describe this)
            if (pWinInfo2->pComment)
                pInfo->maComment = o3tl::toU(pWinInfo2->pComment);
            pInfo->mnStatus = ImplWinQueueStatusToSal(pWinInfo2->Status);
            pInfo->mnJobs = pWinInfo2->cJobs;
            if (!pInfo->moPortName)
                pInfo->moPortName = aPortName;
        }
        std::free(pWinInfo2);
    }
    ClosePrinter(hPrinter);
}

OUString WindowsInstance::GetDefaultPrinter()
{
    DWORD nChars = 0;
    GetDefaultPrinterW(nullptr, &nChars);
    if (nChars)
    {
        std::vector<WCHAR> pStr(nChars);
        if (GetDefaultPrinterW(pStr.data(), &nChars))
            return OUString(o3tl::toU(pStr.data()));
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
