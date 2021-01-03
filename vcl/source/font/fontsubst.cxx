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

#include <vcl/event.hxx>
#include <vcl/outdev.hxx>

#include <window.h>
#include <font/fontsubst.hxx>
#include <font/ImplDirectFontSubstitution.hxx>
#include <font/ImplFontSubstEntry.hxx>
#include <impfontcache.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <PhysicalFontCollection.hxx>

void BeginFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maGDIData.mbFontSubChanged = false;
}

void EndFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->maGDIData.mbFontSubChanged)
    {
        ImplUpdateAllFontData(false);

        DataChangedEvent aDCEvt(DataChangedEventType::FONTSUBSTITUTION);
        Application::ImplCallEventListenersApplicationDataChanged(&aDCEvt);
        Application::NotifyAllWindows(aDCEvt);
        pSVData->maGDIData.mbFontSubChanged = false;
    }
}

void AddFontSubstitute(const OUString& rFontName, const OUString& rReplaceFontName,
                       AddFontSubstituteFlags nFlags)
{
    ImplDirectFontSubstitution*& rpSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if (!rpSubst)
        rpSubst = new ImplDirectFontSubstitution;
    rpSubst->AddFontSubstitute(rFontName, rReplaceFontName, nFlags);
    ImplGetSVData()->maGDIData.mbFontSubChanged = true;
}

void RemoveFontsSubstitute()
{
    ImplDirectFontSubstitution* pSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if (pSubst)
        pSubst->RemoveFontsSubstitute();
}

void ImplUpdateAllFontData(bool bNewFontLists)
{
    ImplClearAllFontData(bNewFontLists);
    ImplRefreshAllFontData(bNewFontLists);
}

void ImplClearAllFontData(bool bNewFontLists)
{
    ImplSVData* pSVData = ImplGetSVData();

    UpdateFontDataForAllFrames(&OutputDevice::ImplClearFontData, bNewFontLists);

    // clear global font lists to have them updated
    pSVData->maGDIData.mxScreenFontCache->Invalidate();
    if (!bNewFontLists)
        return;

    pSVData->maGDIData.mxScreenFontList->Clear();
    vcl::Window* pFrame = pSVData->maFrameData.mpFirstFrame;
    if (pFrame)
    {
        if (pFrame->AcquireGraphics())
        {
            OutputDevice* pDevice = pFrame;
            pDevice->mpGraphics->ClearDevFontCache();
            pDevice->mpGraphics->GetDevFontList(
                pFrame->mpWindowImpl->mpFrameData->mxFontCollection.get());
        }
    }
}

void ImplRefreshAllFontData(bool bNewFontLists)
{
    UpdateFontDataForAllFrames(&OutputDevice::RefreshFontData, bNewFontLists);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
