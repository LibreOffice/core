/*************************************************************************
 *
 *  $RCSfile: svdviter.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrViewIter::ImpInitVars()
{
    nListenerNum=0;
    nPageViewNum=0;
    nOutDevNum=0;
    pAktView=NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrViewIter::SdrViewIter(const SdrModel* pModel_)
{
    pModel=pModel_;
    pPage=NULL;
    pObject=NULL;
    ImpInitVars();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrViewIter::SdrViewIter(const SdrPage* pPage_, FASTBOOL bNoMasterPage_)
{
    pPage=pPage_;
    pModel=pPage_!=NULL ? pPage_->GetModel() : NULL;
    pObject=NULL;
    bNoMasterPage=bNoMasterPage_;
    ImpInitVars();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrViewIter::SdrViewIter(const SdrObject* pObject_, FASTBOOL bNoMasterPage_)
{
    pObject=pObject_;
    pModel=pObject_!=NULL ? pObject_->GetModel() : NULL;
    pPage=pObject_!=NULL ? pObject_->GetPage() : NULL;
    bNoMasterPage=bNoMasterPage_;
    if (pModel==NULL || pPage==NULL) {
        pModel=NULL;
        pPage=NULL;
    }
    ImpInitVars();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrViewIter::ImpCheckPageView(SdrPageView* pPV) const
{
    if (pPage!=NULL) {
        FASTBOOL bMaster=pPage->IsMasterPage();
        USHORT nPageNum=pPage->GetPageNum();
        SdrPage* pPg=pPV->GetPage();
        if (pPg==pPage) {
            if (pObject!=NULL) {
                // Objekt gewuenscht? Na dann erstmal sehen, ob
                // das Obj in dieser PageView auch sichtbar ist.
                SetOfByte aObjLay;
                pObject->GetLayer(aObjLay);
                aObjLay&=pPV->GetVisibleLayers();
                return !aObjLay.IsEmpty();
            } else {
                return TRUE;
            }
        } else {
            if (!bNoMasterPage && bMaster && (pObject==NULL || !pObject->IsNotVisibleAsMaster())) {
                USHORT nMasterPageAnz=pPg->GetMasterPageCount();
                USHORT nMasterPagePos=0;
                while (nMasterPagePos<nMasterPageAnz) {
                    if (nPageNum==pPg->GetMasterPageNum(nMasterPagePos)) {
                        // Aha, die gewuenschte Page ist also MasterPage in dieser PageView
                        if (pObject!=NULL) {
                            // Objekt gewuenscht? Na dann erstmal sehen, ob
                            // das Obj in dieser PageView auch sichtbar ist.
                            SetOfByte aObjLay;
                            pObject->GetLayer(aObjLay);
                            aObjLay&=pPV->GetVisibleLayers();
                            aObjLay&=pPg->GetMasterPageVisibleLayers(nMasterPagePos);
                            if (!aObjLay.IsEmpty()) {
                                return TRUE;
                            } // ansonsten die naechste MasterPage der Page ansehen...
                        } else {
                            return TRUE;
                        }
                    }
                    nMasterPagePos++;
                }
            }
            // MasterPage nicht erlaubt oder keine passende gefunden
            return FALSE;
        }
    } else {
        return TRUE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrView* SdrViewIter::ImpFindView()
{
    if (pModel!=NULL) {
        USHORT nLsAnz=pModel->GetListenerCount();
        while (nListenerNum<nLsAnz) {
            SfxListener* pLs=pModel->GetListener(nListenerNum);
            pAktView=PTR_CAST(SdrView,pLs);
            if (pAktView!=NULL) {
                if (pPage!=NULL) {
                    USHORT nPvAnz=pAktView->GetPageViewCount();
                    USHORT nPvNum=0;
                    while (nPvNum<nPvAnz) {
                        SdrPageView* pPV=pAktView->GetPageViewPvNum(nPvNum);
                        if (ImpCheckPageView(pPV)) {
                            return pAktView;
                        }
                        nPvNum++;
                    }
                } else {
                    return pAktView;
                }
            }
            nListenerNum++;
        }
    }
    pAktView=NULL;
    return pAktView;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPageView* SdrViewIter::ImpFindPageView()
{
    SdrView* pRet=NULL;
    if (pModel!=NULL) {
        while (pAktView!=NULL) {
            USHORT nPvAnz=pAktView->GetPageViewCount();
            while (nPageViewNum<nPvAnz) {
                SdrPageView* pPV=pAktView->GetPageViewPvNum(nPageViewNum);
                if (pPage!=NULL) {
                    if (ImpCheckPageView(pPV)) {
                        return pPV;
                    }
                } else {
                    return pPV;
                }
                nPageViewNum++;
            }
            nListenerNum++;
            ImpFindView();
        }
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OutputDevice* SdrViewIter::ImpFindOutDev()
{
    while (pAktView!=NULL) {
        USHORT nOutDevAnz=pAktView->GetWinCount();
        if (nOutDevNum<nOutDevAnz) {
            return pAktView->GetWin(nOutDevNum);
        }
        nListenerNum++;
        ImpFindView();
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Window* SdrViewIter::ImpFindWindow()
{
    while (pAktView!=NULL) {
        USHORT nOutDevAnz=pAktView->GetWinCount();
        while (nOutDevNum<nOutDevAnz) {
            OutputDevice* pOutDev=pAktView->GetWin(nOutDevNum);
            if (pOutDev->GetOutDevType()==OUTDEV_WINDOW) {
                return (Window*)pOutDev;
            }
            nOutDevNum++;
        }
        nListenerNum++;
        ImpFindView();
    }
    return NULL;
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
    nListenerNum++;
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
    nPageViewNum++;
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
    nOutDevNum++;
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
    nOutDevNum++;
    return ImpFindWindow();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

