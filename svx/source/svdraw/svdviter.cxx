/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "svx/svdviter.hxx"
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdsob.hxx>
#include <svl/brdcst.hxx>
#include <svx/sdrpaintwindow.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrViewIter::ImpInitVars()
{
    mnListenerNum = 0L;
    mnPageViewNum = 0L;
    mnOutDevNum = 0L;
    mpAktView = 0L;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrViewIter::SdrViewIter(const SdrPage* pPage, sal_Bool bNoMasterPage)
{
    mpPage = pPage;
    mpModel = (pPage) ? pPage->GetModel() : 0L;
    mpObject = 0L;
    mbNoMasterPage = bNoMasterPage;
    ImpInitVars();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrViewIter::SdrViewIter(const SdrObject* pObject, sal_Bool bNoMasterPage)
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

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrViewIter::ImpCheckPageView(SdrPageView* pPV) const
{
    if(mpPage)
    {
        sal_Bool bMaster(mpPage->IsMasterPage());
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
                return sal_True;
            }
        }
        else
        {
            if(!mbNoMasterPage && bMaster && (!mpObject || !mpObject->IsNotVisibleAsMaster()))
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
                                return sal_True;
                            } // else, look at the next master page of this page...
                        }
                        else
                        {
                            return sal_True;
                        }
                    }
                }
            }

            // master page forbidden or no fitting master page found
            return sal_False;
        }
    }
    else
    {
        return sal_True;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrView* SdrViewIter::ImpFindView()
{
    if(mpModel)
    {
        sal_uInt32 nLsAnz(mpModel->GetListenerCount());

        while(mnListenerNum < nLsAnz)
        {
            SfxListener* pLs = mpModel->GetListener((sal_uInt16)mnListenerNum);
            mpAktView = PTR_CAST(SdrView, pLs);

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

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrView* SdrViewIter::FirstView()
{
    ImpInitVars();
    return ImpFindView();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrView* SdrViewIter::NextView()
{
    mnListenerNum++;
    return ImpFindView();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
