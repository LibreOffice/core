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

void SdrViewIter::ImpInitVars()
{
    mnListenerNum = 0;
    mpCurrentView = nullptr;
}

SdrViewIter::SdrViewIter(const SdrPage* pPage)
{
    mpPage = pPage;
    mpModel = pPage ? &pPage->getSdrModelFromSdrPage() : nullptr;
    mpObject = nullptr;
    ImpInitVars();
}

SdrViewIter::SdrViewIter(const SdrObject* pObject)
{
    mpObject = pObject;
    mpModel = pObject ? &pObject->getSdrModelFromSdrObject() : nullptr;
    mpPage = pObject ? pObject->getSdrPageFromSdrObject() : nullptr;

    if (!mpModel || !mpPage)
    {
        mpModel = nullptr;
        mpPage = nullptr;
    }

    ImpInitVars();
}

bool SdrViewIter::ImpCheckPageView(SdrPageView const* pPV) const
{
    if (!mpPage)
        return true;

    bool bMaster(mpPage->IsMasterPage());
    SdrPage* pPg = pPV->GetPage();

    if (pPg == mpPage)
    {
        if (mpObject)
        {
            // Looking for an object? First, determine if it visible in
            // this PageView.
            SdrLayerIDSet aObjLay;
            mpObject->getMergedHierarchySdrLayerIDSet(aObjLay);
            aObjLay &= pPV->GetVisibleLayers();
            return !aObjLay.IsEmpty();
        }
        else
        {
            return true;
        }
    }
    else if (bMaster && (!mpObject || !mpObject->IsNotVisibleAsMaster()))
    {
        if (pPg->TRG_HasMasterPage())
        {
            SdrPage& rMasterPage = pPg->TRG_GetMasterPage();

            if (&rMasterPage == mpPage)
            {
                // the page we're looking for is a master page in this PageView
                if (mpObject)
                {
                    // Looking for an object? First, determine if it visible in
                    // this PageView.
                    SdrLayerIDSet aObjLay;
                    mpObject->getMergedHierarchySdrLayerIDSet(aObjLay);
                    aObjLay &= pPV->GetVisibleLayers();
                    aObjLay &= pPg->TRG_GetMasterPageVisibleLayers();

                    if (!aObjLay.IsEmpty())
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

SdrView* SdrViewIter::ImpFindView()
{
    if (mpModel)
    {
        const size_t nLsCnt(mpModel->GetSizeOfVector());

        while (mnListenerNum < nLsCnt)
        {
            SfxListener* pLs = mpModel->GetListener(mnListenerNum);
            mpCurrentView = dynamic_cast<SdrView*>(pLs);

            if (mpCurrentView)
            {
                if (mpPage)
                {
                    SdrPageView* pPV = mpCurrentView->GetSdrPageView();

                    if (pPV && ImpCheckPageView(pPV))
                    {
                        return mpCurrentView;
                    }
                }
                else
                {
                    return mpCurrentView;
                }
            }

            mnListenerNum++;
        }
    }

    mpCurrentView = nullptr;
    return mpCurrentView;
}

SdrView* SdrViewIter::FirstView()
{
    ImpInitVars();
    return ImpFindView();
}

SdrView* SdrViewIter::NextView()
{
    mnListenerNum++;
    return ImpFindView();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
