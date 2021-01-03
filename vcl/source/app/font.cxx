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

#include <window.h>
#include <font/font.hxx>
#include <svdata.hxx>

typedef void (OutputDevice::*FontUpdateHandler_t)(bool);

void UpdateFontDataForAllFrames(const FontUpdateHandler_t pHdl, const bool bNewFontLists)
{
    ImplSVData* const pSVData = ImplGetSVData();

    // update all windows
    vcl::Window* pFrame = pSVData->maFrameData.mpFirstFrame;
    while (pFrame)
    {
        (pFrame->*pHdl)(bNewFontLists);

        vcl::Window* pSysWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while (pSysWin)
        {
            (pSysWin->*pHdl)(bNewFontLists);
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    // update all virtual devices
    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
    while (pVirDev)
    {
        (pVirDev->*pHdl)(bNewFontLists);
        pVirDev = pVirDev->mpNext;
    }

    // update all printers
    Printer* pPrinter = pSVData->maGDIData.mpFirstPrinter;
    while (pPrinter)
    {
        (pPrinter->*pHdl)(bNewFontLists);
        pPrinter = pPrinter->mpNext;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
