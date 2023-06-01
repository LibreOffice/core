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

#include <svx/svdviter.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdsob.hxx>

static bool ImpCheckPageView(const SdrPage* pPage, const SdrObject* pObject, SdrPageView const* pPV)
{
    if (!pPage)
        return true;

    bool bMaster(pPage->IsMasterPage());
    SdrPage* pPg = pPV->GetPage();

    if (pPg == pPage)
    {
        if (pObject)
        {
            // Looking for an object? First, determine if it visible in
            // this PageView.
            return pObject->isVisibleOnAnyOfTheseLayers(pPV->GetVisibleLayers());
        }
        else
        {
            return true;
        }
    }
    else if (bMaster && (!pObject || !pObject->IsNotVisibleAsMaster()))
    {
        if (pPg->TRG_HasMasterPage())
        {
            SdrPage& rMasterPage = pPg->TRG_GetMasterPage();

            if (&rMasterPage == pPage)
            {
                // the page we're looking for is a master page in this PageView
                if (pObject)
                {
                    // Looking for an object? First, determine if it visible in
                    // this PageView.
                    SdrLayerIDSet aObjLay = pPV->GetVisibleLayers();
                    aObjLay &= pPg->TRG_GetMasterPageVisibleLayers();
                    if (pObject->isVisibleOnAnyOfTheseLayers(aObjLay))
                    {
                        return true;
                    } // else, look at the next master page of this page...
                }
                else
                {
                    return true;
                }
            }
        }
    }

    // master page forbidden or no fitting master page found
    return false;
}

namespace SdrViewIter
{
void ForAllViews(const SdrPage* pPage, std::function<void(SdrView*)> f)
{
    if (!pPage)
        return;
    const SdrModel* pModel = &pPage->getSdrModelFromSdrPage();

    pModel->ForAllListeners([&pPage, &f](SfxListener* pLs) {
        if (!pLs->IsSdrView())
            return false;
        SdrView* pCurrentView = static_cast<SdrView*>(pLs);
        SdrPageView* pPV = pCurrentView->GetSdrPageView();

        if (pPV && ImpCheckPageView(pPage, nullptr, pPV))
        {
            f(pCurrentView);
        }
        return false;
    });
}

void ForAllViews(const SdrObject* pObject, std::function<void(SdrView*)> f)
{
    if (!pObject)
        return;
    const SdrModel* pModel = &pObject->getSdrModelFromSdrObject();
    const SdrPage* pPage = pObject->getSdrPageFromSdrObject();
    if (!pPage)
        return;

    pModel->ForAllListeners([&pPage, &pObject, &f](SfxListener* pLs) {
        if (!pLs->IsSdrView())
            return false;
        SdrView* pCurrentView = static_cast<SdrView*>(pLs);
        SdrPageView* pPV = pCurrentView->GetSdrPageView();

        if (pPV && ImpCheckPageView(pPage, pObject, pPV))
        {
            f(pCurrentView);
        }
        return false;
    });
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
