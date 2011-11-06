/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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

SdrViewIter::SdrViewIter(const SdrModel* pModel)
{
    mpModel = pModel;
    mpPage = 0L;
    mpObject = 0L;
    ImpInitVars();
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
                // Objekt gewuenscht? Na dann erstmal sehen, ob
                // das Obj in dieser PageView auch sichtbar ist.
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
                        // Aha, die gewuenschte Page ist also MasterPage in dieser PageView
                        if(mpObject)
                        {
                            // Objekt gewuenscht? Na dann erstmal sehen, ob
                            // das Obj in dieser PageView auch sichtbar ist.
                            SetOfByte aObjLay;
                            mpObject->getMergedHierarchyLayerSet(aObjLay);
                            aObjLay &= pPV->GetVisibleLayers();
                            aObjLay &= pPg->TRG_GetMasterPageVisibleLayers();

                            if(!aObjLay.IsEmpty())
                            {
                                return sal_True;
                            } // ansonsten die naechste MasterPage der Page ansehen...
                        }
                        else
                        {
                            return sal_True;
                        }
                    }
                }
            }

            // MasterPage nicht erlaubt oder keine passende gefunden
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

SdrPageView* SdrViewIter::ImpFindPageView()
{
    if(mpModel)
    {
        while(mpAktView)
        {
            SdrPageView* pPV = mpAktView->GetSdrPageView();

            if(pPV)
            {
                if(mpPage)
                {
                    if(ImpCheckPageView(pPV))
                    {
                        return pPV;
                    }
                }
                else
                {
                    return pPV;
                }

                mnPageViewNum++;
            }

            mnListenerNum++;
            ImpFindView();
        }
    }

    return 0L;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OutputDevice* SdrViewIter::ImpFindOutDev()
{
    while(mpAktView)
    {
        const sal_uInt32 nOutDevAnz(mpAktView->PaintWindowCount());

        if(mnOutDevNum < nOutDevAnz)
        {
            SdrPaintWindow* pPaintWindow = mpAktView->GetPaintWindow(mnOutDevNum);
            return &pPaintWindow->GetOutputDevice();
        }

        mnListenerNum++;
        ImpFindView();
    }

    return 0L;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Window* SdrViewIter::ImpFindWindow()
{
    while(mpAktView)
    {
        const sal_uInt32 nOutDevAnz(mpAktView->PaintWindowCount());

        while(mnOutDevNum < nOutDevAnz)
        {
            SdrPaintWindow* pPaintWindow = mpAktView->GetPaintWindow(mnOutDevNum);
            OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

            if(OUTDEV_WINDOW == rOutDev.GetOutDevType())
            {
                return (Window*)(&rOutDev);
            }

            mnOutDevNum++;
        }

        mnListenerNum++;
        ImpFindView();
    }

    return 0L;
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

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPageView* SdrViewIter::FirstPageView()
{
    ImpInitVars();
    ImpFindView();
    return ImpFindPageView();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPageView* SdrViewIter::NextPageView()
{
    mnPageViewNum++;
    return ImpFindPageView();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OutputDevice* SdrViewIter::FirstOutDev()
{
    ImpInitVars();
    ImpFindView();
    return ImpFindOutDev();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OutputDevice* SdrViewIter::NextOutDev()
{
    mnOutDevNum++;
    return ImpFindOutDev();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Window* SdrViewIter::FirstWindow()
{
    ImpInitVars();
    ImpFindView();
    return ImpFindWindow();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Window* SdrViewIter::NextWindow()
{
    mnOutDevNum++;
    return ImpFindWindow();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

