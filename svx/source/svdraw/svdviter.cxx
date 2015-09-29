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


#include "svx/svdviter.hxx"
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdsob.hxx>
#include <svx/sdrpaintwindow.hxx>



void SdrViewIter::ImpInitVars()
{
    mnListenerNum = 0L;
    mnPageViewNum = 0L;
    mnOutDevNum = 0L;
    mpAktView = 0L;
}



SdrViewIter::SdrViewIter(const SdrPage* pPage, bool bNoMasterPage)
{
    mpPage = pPage;
    mpModel = (pPage) ? pPage->GetModel() : 0L;
    mpObject = 0L;
    mbNoMasterPage = bNoMasterPage;
    ImpInitVars();
}



SdrViewIter::SdrViewIter(const SdrObject* pObject, bool bNoMasterPage)
{
    mpObject = pObject;
    mpModel = (pObject) ? pObject->GetModel() : 0L;
    mpPage = (pObject) ? pObject->GetPage() : 0L;
    mbNoMasterPage = bNoMasterPage;

    if(!mpModel || !mpPage)
    {
        mpModel = 0L;
        mpPage = 0L;
    }

    ImpInitVars();
}



bool SdrViewIter::ImpCheckPageView(SdrPageView* pPV) const
{
    if(!mpPage)
        return true;

    bool bMaster(mpPage->IsMasterPage());
    SdrPage* pPg = pPV->GetPage();

    if(pPg == mpPage)
    {
        if(mpObject)
        {
            // Looking for an object? First, determine if it visible in
            // this PageView.
            SetOfByte aObjLay;
            mpObject->getMergedHierarchyLayerSet(aObjLay);
            aObjLay &= pPV->GetVisibleLayers();
            return !aObjLay.IsEmpty();
        }
        else
        {
            return true;
        }
    }
    else if(!mbNoMasterPage && bMaster && (!mpObject || !mpObject->IsNotVisibleAsMaster()))
    {
        if(pPg->TRG_HasMasterPage())
        {
            SdrPage& rMasterPage = pPg->TRG_GetMasterPage();

            if(&rMasterPage == mpPage)
            {
                // the page we're looking for is a master page in this PageView
                if(mpObject)
                {
                    // Looking for an object? First, determine if it visible in
                    // this PageView.
                    SetOfByte aObjLay;
                    mpObject->getMergedHierarchyLayerSet(aObjLay);
                    aObjLay &= pPV->GetVisibleLayers();
                    aObjLay &= pPg->TRG_GetMasterPageVisibleLayers();

                    if(!aObjLay.IsEmpty())
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
    if(mpModel)
    {
        const size_t nLsAnz(mpModel->GetSizeOfVector());

        while(mnListenerNum < nLsAnz)
        {
            SfxListener* pLs = mpModel->GetListener(mnListenerNum);
            mpAktView = dynamic_cast<SdrView*>( pLs );

            if(mpAktView)
            {
                if(mpPage)
                {
                    SdrPageView* pPV = mpAktView->GetSdrPageView();

                    if(pPV)
                    {
                        if(ImpCheckPageView(pPV))
                        {
                            return mpAktView;
                        }
                    }
                }
                else
                {
                    return mpAktView;
                }
            }

            mnListenerNum++;
        }
    }

    mpAktView = 0L;
    return mpAktView;
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
