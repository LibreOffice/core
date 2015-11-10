/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <svdobjplusdata.hxx>
#include <svdobjuserdatalist.hxx>

#include <svx/svdglue.hxx>

#include <svl/SfxBroadcaster.hxx>
#include <vcl/outdev.hxx>

SdrObjPlusData::SdrObjPlusData():
    pBroadcast(nullptr),
    pUserDataList(nullptr),
    pGluePoints(nullptr)
{
}

SdrObjPlusData::~SdrObjPlusData()
{
    delete pBroadcast;
    delete pUserDataList;
    delete pGluePoints;
}

SdrObjPlusData* SdrObjPlusData::Clone(SdrObject* pObj1) const
{
    SdrObjPlusData* pNeuPlusData=new SdrObjPlusData;
    if (pUserDataList!=nullptr) {
        sal_uInt16 nCount=pUserDataList->GetUserDataCount();
        if (nCount!=0) {
            pNeuPlusData->pUserDataList=new SdrObjUserDataList;
            for (sal_uInt16 i=0; i<nCount; i++) {
                SdrObjUserData* pNeuUserData=pUserDataList->GetUserData(i).Clone(pObj1);
                if (pNeuUserData!=nullptr) {
                    pNeuPlusData->pUserDataList->AppendUserData(pNeuUserData);
                } else {
                    OSL_FAIL("SdrObjPlusData::Clone(): UserData.Clone() returns NULL.");
                }
            }
        }
    }
    if (pGluePoints!=nullptr) pNeuPlusData->pGluePoints=new SdrGluePointList(*pGluePoints);
    // MtfAnimator isn't copied either

    // #i68101#
    // copy object name, title and description
    pNeuPlusData->aObjName = aObjName;
    pNeuPlusData->aObjTitle = aObjTitle;
    pNeuPlusData->aObjDescription = aObjDescription;

    return pNeuPlusData;
}

void SdrObjPlusData::SetGluePoints(const SdrGluePointList& rPts)
{
    return *pGluePoints = rPts;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
