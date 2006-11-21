/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdviter.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 16:57:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svdviter.hxx"
#include "svdobj.hxx"
#include "svdpage.hxx"
#include "svdmodel.hxx"
#include "svdview.hxx"
#include "svdpagv.hxx"
#include "svdsob.hxx"

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

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
                pObject->getMergedHierarchyLayerSet(aObjLay);
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
                            pObject->getMergedHierarchyLayerSet(aObjLay);
                            aObjLay &= pPV->GetVisibleLayers();
                            aObjLay &= pPg->TRG_GetMasterPageVisibleLayers();

                            if(!aObjLay.IsEmpty())
                            {
                                return TRUE;
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

